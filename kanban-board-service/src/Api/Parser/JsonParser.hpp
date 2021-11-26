#pragma once

#include "ParserIf.hpp"
#include "rapidjson/document.h"

namespace Prog3 {
namespace Api {
namespace Parser {

class JsonParser : public ParserIf {
  private:
    static inline std::string const EMPTY_JSON = "{}";

    bool isValidColumn(rapidjson::Document const &document);
    bool isValidItem(rapidjson::Document const &document);

    std::string jsonValueToString(rapidjson::Value const &json);

    rapidjson::Value getJsonValueFromModel(Prog3::Core::Model::Item const &item, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value getJsonValueFromModel(Prog3::Core::Model::Column const &column, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value getJsonValueFromModel(Prog3::Core::Model::Board &board, rapidjson::Document::AllocatorType &allocator);

  public:
    JsonParser(){};
    virtual ~JsonParser(){};

    virtual std::string convertToApiString(Prog3::Core::Model::Board &board);

    virtual std::string convertToApiString(Prog3::Core::Model::Column &column);
    virtual std::string convertToApiString(std::vector<Prog3::Core::Model::Column> &columns);

    virtual std::string convertToApiString(Prog3::Core::Model::Item &item);
    virtual std::string convertToApiString(std::vector<Prog3::Core::Model::Item> &items);

    virtual std::optional<Prog3::Core::Model::Column> convertColumnToModel(int columnId, std::string &request);
    virtual std::optional<Prog3::Core::Model::Item> convertItemToModel(int itemId, std::string &request);

    virtual std::string getEmptyResponseString() {
        return JsonParser::EMPTY_JSON;
    }
};

} // namespace Parser
} // namespace Api
} // namespace Prog3