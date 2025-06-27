#include <qpdf/SF_FlateLzwDecode.hh>

#include <qpdf/Pl_Flate.hh>
#include <qpdf/Pl_LZWDecoder.hh>
#include <qpdf/Pl_PNGFilter.hh>
#include <qpdf/Pl_TIFFPredictor.hh>
#include <qpdf/QIntC.hh>
#include <qpdf/QTC.hh>

bool
SF_FlateLzwDecode::setDecodeParms(QPDFObjectHandle decode_parms)
{
    if (decode_parms.isNull()) {
        return true;
    }

    auto memory_limit = Pl_Flate::memory_limit();

    std::set<std::string> keys = decode_parms.getKeys();
    for (auto const& key: keys) {
        QPDFObjectHandle value = decode_parms.getKey(key);
        if (key == "/Predictor") {
            if (value.isInteger()) {
                predictor = value.getIntValueAsInt();
                if (!(predictor == 1 || predictor == 2 || (predictor >= 10 && predictor <= 15))) {
                    return false;
                }
            } else {
                return false;
            }
        } else if (key == "/Columns" || key == "/Colors" || key == "/BitsPerComponent") {
            if (value.isInteger()) {
                int val = value.getIntValueAsInt();
                if (memory_limit && static_cast<unsigned int>(val) > memory_limit) {
                    QPDFLogger::defaultLogger()->warn(
                        "SF_FlateLzwDecode parameter exceeds PL_Flate memory limit\n");
                    return false;
                }
                if (key == "/Columns") {
                    columns = val;
                } else if (key == "/Colors") {
                    colors = val;
                } else if (key == "/BitsPerComponent") {
                    bits_per_component = val;
                }
            } else {
                return false;
            }
        } else if (lzw && (key == "/EarlyChange")) {
            if (value.isInteger()) {
                int earlychange = value.getIntValueAsInt();
                early_code_change = (earlychange == 1);
                if (!(earlychange == 0 || earlychange == 1)) {
                    return false;
                }
            } else {
                return false;
            }
        }
    }

    if (predictor > 1 && columns == 0) {
        return false;
    }

    return true;
}

Pipeline*
SF_FlateLzwDecode::getDecodePipeline(Pipeline* next)
{
    std::shared_ptr<Pipeline> pipeline;
    if (predictor >= 10 && predictor <= 15) {
        QTC::TC("qpdf", "SF_FlateLzwDecode PNG filter");
        pipeline = std::make_shared<Pl_PNGFilter>(
            "png decode",
            next,
            Pl_PNGFilter::a_decode,
            QIntC::to_uint(columns),
            QIntC::to_uint(colors),
            QIntC::to_uint(bits_per_component));
        pipelines.push_back(pipeline);
        next = pipeline.get();
    } else if (predictor == 2) {
        QTC::TC("qpdf", "SF_FlateLzwDecode TIFF predictor");
        pipeline = std::make_shared<Pl_TIFFPredictor>(
            "tiff decode",
            next,
            Pl_TIFFPredictor::a_decode,
            QIntC::to_uint(columns),
            QIntC::to_uint(colors),
            QIntC::to_uint(bits_per_component));
        pipelines.push_back(pipeline);
        next = pipeline.get();
    }

    if (lzw) {
        pipeline = std::make_shared<Pl_LZWDecoder>("lzw decode", next, early_code_change);
    } else {
        pipeline = std::make_shared<Pl_Flate>("stream inflate", next, Pl_Flate::a_inflate);
    }
    pipelines.push_back(pipeline);
    return pipeline.get();
}

std::shared_ptr<QPDFStreamFilter>
SF_FlateLzwDecode::flate_factory()
{
    return std::make_shared<SF_FlateLzwDecode>(false);
}

std::shared_ptr<QPDFStreamFilter>
SF_FlateLzwDecode::lzw_factory()
{
    return std::make_shared<SF_FlateLzwDecode>(true);
}
