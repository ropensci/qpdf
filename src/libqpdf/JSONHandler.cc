#include <qpdf/JSONHandler.hh>

#include <qpdf/QPDFUsage.hh>
#include <qpdf/QTC.hh>
#include <qpdf/QUtil.hh>

struct Handlers
{
    Handlers() = default;

    JSONHandler::json_handler_t any_handler{nullptr};
    JSONHandler::void_handler_t null_handler{nullptr};
    JSONHandler::string_handler_t string_handler{nullptr};
    JSONHandler::string_handler_t number_handler{nullptr};
    JSONHandler::bool_handler_t bool_handler{nullptr};
    JSONHandler::json_handler_t dict_start_handler{nullptr};
    JSONHandler::void_handler_t dict_end_handler{nullptr};
    JSONHandler::json_handler_t array_start_handler{nullptr};
    JSONHandler::void_handler_t array_end_handler{nullptr};
    std::map<std::string, std::shared_ptr<JSONHandler>> dict_handlers;
    std::shared_ptr<JSONHandler> fallback_dict_handler;
    std::shared_ptr<JSONHandler> array_item_handler;
    std::shared_ptr<JSONHandler> fallback_handler;
};

class JSONHandler::Members
{
    friend class JSONHandler;

  public:
    ~Members() = default;

  private:
    Members() = default;
    Members(Members const&) = delete;

    Handlers h;
};

JSONHandler::JSONHandler() :
    m(new Members())
{
}

JSONHandler::~JSONHandler()
{
}

void
JSONHandler::usage(std::string const& msg)
{
    throw QPDFUsage(msg);
}

void
JSONHandler::addAnyHandler(json_handler_t fn)
{
    m->h.any_handler = fn;
}

void
JSONHandler::addNullHandler(void_handler_t fn)
{
    m->h.null_handler = fn;
}

void
JSONHandler::addStringHandler(string_handler_t fn)
{
    m->h.string_handler = fn;
}

void
JSONHandler::addNumberHandler(string_handler_t fn)
{
    m->h.number_handler = fn;
}

void
JSONHandler::addBoolHandler(bool_handler_t fn)
{
    m->h.bool_handler = fn;
}

void
JSONHandler::addDictHandlers(json_handler_t start_fn, void_handler_t end_fn)
{
    m->h.dict_start_handler = start_fn;
    m->h.dict_end_handler = end_fn;
}

void
JSONHandler::addDictKeyHandler(std::string const& key, std::shared_ptr<JSONHandler> dkh)
{
    m->h.dict_handlers[key] = dkh;
}

void
JSONHandler::addFallbackDictHandler(std::shared_ptr<JSONHandler> fdh)
{
    m->h.fallback_dict_handler = fdh;
}

void
JSONHandler::addArrayHandlers(
    json_handler_t start_fn, void_handler_t end_fn, std::shared_ptr<JSONHandler> ah)
{
    m->h.array_start_handler = start_fn;
    m->h.array_end_handler = end_fn;
    m->h.array_item_handler = ah;
}

void
JSONHandler::addFallbackHandler(std::shared_ptr<JSONHandler> h)
{
    m->h.fallback_handler = std::move(h);
}

void
JSONHandler::handle(std::string const& path, JSON j)
{
    if (m->h.any_handler) {
        m->h.any_handler(path, j);
        return;
    }

    bool bvalue = false;
    std::string s_value;
    if (m->h.null_handler && j.isNull()) {
        m->h.null_handler(path);
        return;
    }
    if (m->h.string_handler && j.getString(s_value)) {
        m->h.string_handler(path, s_value);
        return;
    }
    if (m->h.number_handler && j.getNumber(s_value)) {
        m->h.number_handler(path, s_value);
        return;
    }
    if (m->h.bool_handler && j.getBool(bvalue)) {
        m->h.bool_handler(path, bvalue);
        return;
    }
    if (m->h.dict_start_handler && j.isDictionary()) {
        m->h.dict_start_handler(path, j);
        std::string path_base = path;
        if (path_base != ".") {
            path_base += ".";
        }
        j.forEachDictItem([&path, &path_base, this](std::string const& k, JSON v) {
            auto i = m->h.dict_handlers.find(k);
            if (i == m->h.dict_handlers.end()) {
                if (m->h.fallback_dict_handler.get()) {
                    m->h.fallback_dict_handler->handle(path_base + k, v);
                } else {
                    QTC::TC("libtests", "JSONHandler unexpected key");
                    usage("JSON handler found unexpected key " + k + " in object at " + path);
                }
            } else {
                i->second->handle(path_base + k, v);
            }
        });
        m->h.dict_end_handler(path);
        return;
    }
    if (m->h.array_start_handler && j.isArray()) {
        m->h.array_start_handler(path, j);
        size_t i = 0;
        j.forEachArrayItem([&i, &path, this](JSON v) {
            m->h.array_item_handler->handle(path + "[" + std::to_string(i) + "]", v);
            ++i;
        });
        m->h.array_end_handler(path);
        return;
    }

    if (m->h.fallback_handler) {
        m->h.fallback_handler->handle(path, j);
        return;
    }

    // It would be nice to include information about what type the object was and what types were
    // allowed, but we're relying on schema validation to make sure input is properly structured
    // before calling the handlers. It would be different if this code were trying to be part of a
    // general-purpose JSON package.
    QTC::TC("libtests", "JSONHandler unhandled value");
    usage("JSON handler: value at " + path + " is not of expected type");
}
