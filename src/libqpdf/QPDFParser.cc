#include <qpdf/QPDFParser.hh>

#include <qpdf/BufferInputSource.hh>
#include <qpdf/QPDF.hh>
#include <qpdf/QPDFObjGen.hh>
#include <qpdf/QPDFObjectHandle.hh>
#include <qpdf/QPDFObject_private.hh>
#include <qpdf/QPDFTokenizer_private.hh>
#include <qpdf/QTC.hh>
#include <qpdf/QUtil.hh>

#include <memory>

using namespace std::literals;
using namespace qpdf;

using ObjectPtr = std::shared_ptr<QPDFObject>;

QPDFObjectHandle
QPDFParser::parse(InputSource& input, std::string const& object_description, QPDF* context)
{
    qpdf::Tokenizer tokenizer;
    bool empty = false;
    return QPDFParser(
               input,
               make_description(input.getName(), object_description),
               object_description,
               tokenizer,
               nullptr,
               context,
               false)
        .parse(empty, false);
}

QPDFObjectHandle
QPDFParser::parse_content(
    InputSource& input,
    std::shared_ptr<QPDFObject::Description> sp_description,
    qpdf::Tokenizer& tokenizer,
    QPDF* context)
{
    bool empty = false;
    return QPDFParser(
               input, std::move(sp_description), "content", tokenizer, nullptr, context, true)
        .parse(empty, true);
}

QPDFObjectHandle
QPDFParser::parse(
    InputSource& input,
    std::string const& object_description,
    QPDFTokenizer& tokenizer,
    bool& empty,
    QPDFObjectHandle::StringDecrypter* decrypter,
    QPDF* context)
{
    return QPDFParser(
               input,
               make_description(input.getName(), object_description),
               object_description,
               *tokenizer.m,
               decrypter,
               context,
               false)
        .parse(empty, false);
}

std::pair<QPDFObjectHandle, bool>
QPDFParser::parse(
    InputSource& input,
    std::string const& object_description,
    qpdf::Tokenizer& tokenizer,
    QPDFObjectHandle::StringDecrypter* decrypter,
    QPDF& context,
    bool sanity_checks)
{
    bool empty{false};
    auto result = QPDFParser(
                      input,
                      make_description(input.getName(), object_description),
                      object_description,
                      tokenizer,
                      decrypter,
                      &context,
                      true,
                      0,
                      0,
                      sanity_checks)
                      .parse(empty, false);
    return {result, empty};
}

std::pair<QPDFObjectHandle, bool>
QPDFParser::parse(
    is::OffsetBuffer& input, int stream_id, int obj_id, qpdf::Tokenizer& tokenizer, QPDF& context)
{
    bool empty{false};
    auto result = QPDFParser(
                      input,
                      std::make_shared<QPDFObject::Description>(
                          QPDFObject::ObjStreamDescr(stream_id, obj_id)),
                      "",
                      tokenizer,
                      nullptr,
                      &context,
                      true,
                      stream_id,
                      obj_id)
                      .parse(empty, false);
    return {result, empty};
}

QPDFObjectHandle
QPDFParser::parse(bool& empty, bool content_stream)
{
    // This method must take care not to resolve any objects. Don't check the type of any object
    // without first ensuring that it is a direct object. Otherwise, doing so may have the side
    // effect of reading the object and changing the file pointer. If you do this, it will cause a
    // logic error to be thrown from QPDF::inParse().

    QPDF::ParseGuard pg(context);
    empty = false;
    start = input.tell();

    if (!tokenizer.nextToken(input, object_description)) {
        warn(tokenizer.getErrorMessage());
    }

    switch (tokenizer.getType()) {
    case QPDFTokenizer::tt_eof:
        if (content_stream) {
            // In content stream mode, leave object uninitialized to indicate EOF
            return {};
        }
        QTC::TC("qpdf", "QPDFParser eof in parse");
        warn("unexpected EOF");
        return {QPDFObject::create<QPDF_Null>()};

    case QPDFTokenizer::tt_bad:
        QTC::TC("qpdf", "QPDFParser bad token in parse");
        return {QPDFObject::create<QPDF_Null>()};

    case QPDFTokenizer::tt_brace_open:
    case QPDFTokenizer::tt_brace_close:
        QTC::TC("qpdf", "QPDFParser bad brace");
        warn("treating unexpected brace token as null");
        return {QPDFObject::create<QPDF_Null>()};

    case QPDFTokenizer::tt_array_close:
        QTC::TC("qpdf", "QPDFParser bad array close");
        warn("treating unexpected array close token as null");
        return {QPDFObject::create<QPDF_Null>()};

    case QPDFTokenizer::tt_dict_close:
        QTC::TC("qpdf", "QPDFParser bad dictionary close");
        warn("unexpected dictionary close token");
        return {QPDFObject::create<QPDF_Null>()};

    case QPDFTokenizer::tt_array_open:
    case QPDFTokenizer::tt_dict_open:
        stack.clear();
        stack.emplace_back(
            input,
            (tokenizer.getType() == QPDFTokenizer::tt_array_open) ? st_array : st_dictionary_key);
        frame = &stack.back();
        return parseRemainder(content_stream);

    case QPDFTokenizer::tt_bool:
        return withDescription<QPDF_Bool>(tokenizer.getValue() == "true");

    case QPDFTokenizer::tt_null:
        return {QPDFObject::create<QPDF_Null>()};

    case QPDFTokenizer::tt_integer:
        return withDescription<QPDF_Integer>(QUtil::string_to_ll(tokenizer.getValue().c_str()));

    case QPDFTokenizer::tt_real:
        return withDescription<QPDF_Real>(tokenizer.getValue());

    case QPDFTokenizer::tt_name:
        return withDescription<QPDF_Name>(tokenizer.getValue());

    case QPDFTokenizer::tt_word:
        {
            auto const& value = tokenizer.getValue();
            if (content_stream) {
                return withDescription<QPDF_Operator>(value);
            } else if (value == "endobj") {
                // We just saw endobj without having read anything.  Treat this as a null and do
                // not move the input source's offset.
                input.seek(input.getLastOffset(), SEEK_SET);
                empty = true;
                return {QPDFObject::create<QPDF_Null>()};
            } else {
                QTC::TC("qpdf", "QPDFParser treat word as string");
                warn("unknown token while reading object; treating as string");
                return withDescription<QPDF_String>(value);
            }
        }

    case QPDFTokenizer::tt_string:
        if (decrypter) {
            std::string s{tokenizer.getValue()};
            decrypter->decryptString(s);
            return withDescription<QPDF_String>(s);
        } else {
            return withDescription<QPDF_String>(tokenizer.getValue());
        }

    default:
        warn("treating unknown token type as null while reading object");
        return {QPDFObject::create<QPDF_Null>()};
    }
}

QPDFObjectHandle
QPDFParser::parseRemainder(bool content_stream)
{
    // This method must take care not to resolve any objects. Don't check the type of any object
    // without first ensuring that it is a direct object. Otherwise, doing so may have the side
    // effect of reading the object and changing the file pointer. If you do this, it will cause a
    // logic error to be thrown from QPDF::inParse().

    bad_count = 0;
    bool b_contents = false;

    while (true) {
        if (!tokenizer.nextToken(input, object_description)) {
            warn(tokenizer.getErrorMessage());
        }
        ++good_count; // optimistically

        if (int_count != 0) {
            // Special handling of indirect references. Treat integer tokens as part of an indirect
            // reference until proven otherwise.
            if (tokenizer.getType() == QPDFTokenizer::tt_integer) {
                if (++int_count > 2) {
                    // Process the oldest buffered integer.
                    addInt(int_count);
                }
                last_offset_buffer[int_count % 2] = input.getLastOffset();
                int_buffer[int_count % 2] = QUtil::string_to_ll(tokenizer.getValue().c_str());
                continue;

            } else if (
                int_count >= 2 && tokenizer.getType() == QPDFTokenizer::tt_word &&
                tokenizer.getValue() == "R") {
                if (context == nullptr) {
                    QTC::TC("qpdf", "QPDFParser indirect without context");
                    throw std::logic_error(
                        "QPDFParser::parse called without context on an object "
                        "with indirect references");
                }
                auto id = QIntC::to_int(int_buffer[(int_count - 1) % 2]);
                auto gen = QIntC::to_int(int_buffer[(int_count) % 2]);
                if (!(id < 1 || gen < 0 || gen >= 65535)) {
                    add(QPDF::ParseGuard::getObject(context, id, gen, parse_pdf));
                } else {
                    QTC::TC("qpdf", "QPDFParser invalid objgen");
                    addNull();
                }
                int_count = 0;
                continue;

            } else if (int_count > 0) {
                // Process the buffered integers before processing the current token.
                if (int_count > 1) {
                    addInt(int_count - 1);
                }
                addInt(int_count);
                int_count = 0;
            }
        }

        switch (tokenizer.getType()) {
        case QPDFTokenizer::tt_eof:
            warn("parse error while reading object");
            if (content_stream) {
                // In content stream mode, leave object uninitialized to indicate EOF
                return {};
            }
            QTC::TC("qpdf", "QPDFParser eof in parseRemainder");
            warn("unexpected EOF");
            return {QPDFObject::create<QPDF_Null>()};

        case QPDFTokenizer::tt_bad:
            QTC::TC("qpdf", "QPDFParser bad token in parseRemainder");
            if (tooManyBadTokens()) {
                return {QPDFObject::create<QPDF_Null>()};
            }
            addNull();
            continue;

        case QPDFTokenizer::tt_brace_open:
        case QPDFTokenizer::tt_brace_close:
            QTC::TC("qpdf", "QPDFParser bad brace in parseRemainder");
            warn("treating unexpected brace token as null");
            if (tooManyBadTokens()) {
                return {QPDFObject::create<QPDF_Null>()};
            }
            addNull();
            continue;

        case QPDFTokenizer::tt_array_close:
            if ((bad_count || sanity_checks) && !max_bad_count) {
                // Trigger warning.
                (void)tooManyBadTokens();
                return {QPDFObject::create<QPDF_Null>()};
            }
            if (frame->state == st_array) {
                auto object = frame->null_count > 100
                    ? QPDFObject::create<QPDF_Array>(std::move(frame->olist), true)
                    : QPDFObject::create<QPDF_Array>(std::move(frame->olist));
                setDescription(object, frame->offset - 1);
                // The `offset` points to the next of "[".  Set the rewind offset to point to the
                // beginning of "[". This has been explicitly tested with whitespace surrounding the
                // array start delimiter. getLastOffset points to the array end token and therefore
                // can't be used here.
                if (stack.size() <= 1) {
                    return object;
                }
                stack.pop_back();
                frame = &stack.back();
                add(std::move(object));
            } else {
                QTC::TC("qpdf", "QPDFParser bad array close in parseRemainder");
                warn("treating unexpected array close token as null");
                if (tooManyBadTokens()) {
                    return {QPDFObject::create<QPDF_Null>()};
                }
                addNull();
            }
            continue;

        case QPDFTokenizer::tt_dict_close:
            if ((bad_count || sanity_checks) && !max_bad_count) {
                // Trigger warning.
                (void)tooManyBadTokens();
                return {QPDFObject::create<QPDF_Null>()};
            }
            if (frame->state <= st_dictionary_value) {
                // Attempt to recover more or less gracefully from invalid dictionaries.
                auto& dict = frame->dict;

                if (frame->state == st_dictionary_value) {
                    QTC::TC("qpdf", "QPDFParser no val for last key");
                    warn(
                        frame->offset,
                        "dictionary ended prematurely; using null as value for last key");
                    dict[frame->key] = QPDFObject::create<QPDF_Null>();
                }

                if (!frame->olist.empty()) {
                    fixMissingKeys();
                }

                if (!frame->contents_string.empty() && dict.count("/Type") &&
                    dict["/Type"].isNameAndEquals("/Sig") && dict.count("/ByteRange") &&
                    dict.count("/Contents") && dict["/Contents"].isString()) {
                    dict["/Contents"] = QPDFObjectHandle::newString(frame->contents_string);
                    dict["/Contents"].setParsedOffset(frame->contents_offset);
                }
                auto object = QPDFObject::create<QPDF_Dictionary>(std::move(dict));
                setDescription(object, frame->offset - 2);
                // The `offset` points to the next of "<<". Set the rewind offset to point to the
                // beginning of "<<". This has been explicitly tested with whitespace surrounding
                // the dictionary start delimiter. getLastOffset points to the dictionary end token
                // and therefore can't be used here.
                if (stack.size() <= 1) {
                    return object;
                }
                stack.pop_back();
                frame = &stack.back();
                add(std::move(object));
            } else {
                QTC::TC("qpdf", "QPDFParser bad dictionary close in parseRemainder");
                warn("unexpected dictionary close token");
                if (tooManyBadTokens()) {
                    return {QPDFObject::create<QPDF_Null>()};
                }
                addNull();
            }
            continue;

        case QPDFTokenizer::tt_array_open:
        case QPDFTokenizer::tt_dict_open:
            if (stack.size() > 499) {
                QTC::TC("qpdf", "QPDFParser too deep");
                warn("ignoring excessively deeply nested data structure");
                return {QPDFObject::create<QPDF_Null>()};
            } else {
                b_contents = false;
                stack.emplace_back(
                    input,
                    (tokenizer.getType() == QPDFTokenizer::tt_array_open) ? st_array
                                                                          : st_dictionary_key);
                frame = &stack.back();
                continue;
            }

        case QPDFTokenizer::tt_bool:
            addScalar<QPDF_Bool>(tokenizer.getValue() == "true");
            continue;

        case QPDFTokenizer::tt_null:
            addNull();
            continue;

        case QPDFTokenizer::tt_integer:
            if (!content_stream) {
                // Buffer token in case it is part of an indirect reference.
                last_offset_buffer[1] = input.getLastOffset();
                int_buffer[1] = QUtil::string_to_ll(tokenizer.getValue().c_str());
                int_count = 1;
            } else {
                addScalar<QPDF_Integer>(QUtil::string_to_ll(tokenizer.getValue().c_str()));
            }
            continue;

        case QPDFTokenizer::tt_real:
            addScalar<QPDF_Real>(tokenizer.getValue());
            continue;

        case QPDFTokenizer::tt_name:
            if (frame->state == st_dictionary_key) {
                frame->key = tokenizer.getValue();
                frame->state = st_dictionary_value;
                b_contents = decrypter && frame->key == "/Contents";
                continue;
            } else {
                addScalar<QPDF_Name>(tokenizer.getValue());
            }
            continue;

        case QPDFTokenizer::tt_word:
            if (content_stream) {
                addScalar<QPDF_Operator>(tokenizer.getValue());
            } else {
                QTC::TC("qpdf", "QPDFParser treat word as string in parseRemainder");
                warn("unknown token while reading object; treating as string");
                if (tooManyBadTokens()) {
                    return {QPDFObject::create<QPDF_Null>()};
                }
                addScalar<QPDF_String>(tokenizer.getValue());
            }
            continue;

        case QPDFTokenizer::tt_string:
            {
                auto const& val = tokenizer.getValue();
                if (decrypter) {
                    if (b_contents) {
                        frame->contents_string = val;
                        frame->contents_offset = input.getLastOffset();
                        b_contents = false;
                    }
                    std::string s{val};
                    decrypter->decryptString(s);
                    addScalar<QPDF_String>(s);
                } else {
                    addScalar<QPDF_String>(val);
                }
            }
            continue;

        default:
            warn("treating unknown token type as null while reading object");
            if (tooManyBadTokens()) {
                return {QPDFObject::create<QPDF_Null>()};
            }
            addNull();
        }
    }
}

void
QPDFParser::add(std::shared_ptr<QPDFObject>&& obj)
{
    if (frame->state != st_dictionary_value) {
        // If state is st_dictionary_key then there is a missing key. Push onto olist for
        // processing once the tt_dict_close token has been found.
        frame->olist.emplace_back(std::move(obj));
    } else {
        if (auto res = frame->dict.insert_or_assign(frame->key, std::move(obj)); !res.second) {
            warnDuplicateKey();
        }
        frame->state = st_dictionary_key;
    }
}

void
QPDFParser::addNull()
{
    const static ObjectPtr null_obj = QPDFObject::create<QPDF_Null>();

    if (frame->state != st_dictionary_value) {
        // If state is st_dictionary_key then there is a missing key. Push onto olist for
        // processing once the tt_dict_close token has been found.
        frame->olist.emplace_back(null_obj);
    } else {
        if (auto res = frame->dict.insert_or_assign(frame->key, null_obj); !res.second) {
            warnDuplicateKey();
        }
        frame->state = st_dictionary_key;
    }
    ++frame->null_count;
}

void
QPDFParser::addInt(int count)
{
    auto obj = QPDFObject::create<QPDF_Integer>(int_buffer[count % 2]);
    obj->setDescription(context, description, last_offset_buffer[count % 2]);
    add(std::move(obj));
}

template <typename T, typename... Args>
void
QPDFParser::addScalar(Args&&... args)
{
    if ((bad_count || sanity_checks) &&
        (frame->olist.size() > 5'000 || frame->dict.size() > 5'000)) {
        // Stop adding scalars. We are going to abort when the close token or a bad token is
        // encountered.
        max_bad_count = 0;
        return;
    }
    auto obj = QPDFObject::create<T>(std::forward<Args>(args)...);
    obj->setDescription(context, description, input.getLastOffset());
    add(std::move(obj));
}

template <typename T, typename... Args>
QPDFObjectHandle
QPDFParser::withDescription(Args&&... args)
{
    auto obj = QPDFObject::create<T>(std::forward<Args>(args)...);
    obj->setDescription(context, description, start);
    return {obj};
}

void
QPDFParser::setDescription(ObjectPtr& obj, qpdf_offset_t parsed_offset)
{
    if (obj) {
        obj->setDescription(context, description, parsed_offset);
    }
}

void
QPDFParser::fixMissingKeys()
{
    std::set<std::string> names;
    for (auto& obj: frame->olist) {
        if (obj.getObj()->getTypeCode() == ::ot_name) {
            names.insert(obj.getObj()->getStringValue());
        }
    }
    int next_fake_key = 1;
    for (auto const& item: frame->olist) {
        while (true) {
            const std::string key = "/QPDFFake" + std::to_string(next_fake_key++);
            const bool found_fake = frame->dict.count(key) == 0 && names.count(key) == 0;
            QTC::TC("qpdf", "QPDFParser found fake", (found_fake ? 0 : 1));
            if (found_fake) {
                warn(
                    frame->offset,
                    "expected dictionary key but found non-name object; inserting key " + key);
                frame->dict[key] = item;
                break;
            }
        }
    }
}

bool
QPDFParser::tooManyBadTokens()
{
    if (frame->olist.size() > 5'000 || frame->dict.size() > 5'000) {
        if (bad_count) {
            warn(
                "encountered errors while parsing an array or dictionary with more than 5000 "
                "elements; giving up on reading object");
            return true;
        }
        warn(
            "encountered an array or dictionary with more than 5000 elements during xref recovery; "
            "giving up on reading object");
    }
    if (--max_bad_count > 0 && good_count > 4) {
        good_count = 0;
        bad_count = 1;
        return false;
    }
    if (++bad_count > 5 ||
        (frame->state != st_array && QIntC::to_size(max_bad_count) < frame->olist.size())) {
        // Give up after 5 errors in close proximity or if the number of missing dictionary keys
        // exceeds the remaining number of allowable total errors.
        warn("too many errors; giving up on reading object");
        return true;
    }
    good_count = 0;
    return false;
}

void
QPDFParser::warn(QPDFExc const& e) const
{
    // If parsing on behalf of a QPDF object and want to give a warning, we can warn through the
    // object. If parsing for some other reason, such as an explicit creation of an object from a
    // string, then just throw the exception.
    if (context) {
        context->warn(e);
    } else {
        throw e;
    }
}

void
QPDFParser::warnDuplicateKey()
{
    QTC::TC("qpdf", "QPDFParser duplicate dict key");
    warn(
        frame->offset,
        "dictionary has duplicated key " + frame->key + "; last occurrence overrides earlier ones");
}

void
QPDFParser::warn(qpdf_offset_t offset, std::string const& msg) const
{
    if (stream_id) {
        std::string descr = "object "s + std::to_string(obj_id) + " 0";
        std::string name = context->getFilename() + " object stream " + std::to_string(stream_id);
        warn(QPDFExc(qpdf_e_damaged_pdf, name, descr, offset, msg));
    } else {
        warn(QPDFExc(qpdf_e_damaged_pdf, input.getName(), object_description, offset, msg));
    }
}

void
QPDFParser::warn(std::string const& msg) const
{
    warn(input.getLastOffset(), msg);
}
