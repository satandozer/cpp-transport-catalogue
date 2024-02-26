#include "json.h"
#include <iomanip>

using namespace std;

namespace json {

//----------- Load ----------
namespace load{

std::string ClearLine (const std::string& str) {
    size_t start = 0, end = str.length();

    while (start < str.length() && 
        (str[start] == '\n' || 
        str[start] == '\r' || 
        str[start] == '\t' || 
        str[start] == ' ')) {
        start++;
    }
    while (end > 0 && 
        (str[end - 1] == '\n' || 
        str[end - 1] == '\r' || 
        str[end - 1] == '\t' || 
        str[end - 1] == ' ')) {
        end--;
    }
    return str.substr(start, end - start);
}    

std::string FindEndOfLine (istream& input) {
    std::string result;
    char ch;
    while (input.get(ch)) {
        if (ch == '}' || ch == ',' || ch == ']') {
            input.putback(ch);
            break;
        }
        result += ch;
    }
    return result;
}

json::Node Node(istream& input);

json::Node Null(istream& input){
    std::string line;
    line = FindEndOfLine(input);
    line = ClearLine(line);
    if (line != "null") {
        throw ParsingError("Error while parsing JSON, wrong argument - null");
    }
    return json::Node();
}

json::Node Array(istream& input) {
    json::Array result;
    if (input.peek() == -1) {
        throw ParsingError("Unexpected end of input");
    }
    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(load::Node(input));
    }
    return json::Node(move(result));
}

using Number = std::variant<int, double>;

Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

json::Node Num(istream& input) {
    Number parsed = LoadNumber(input);
    
    if (std::holds_alternative<int>(parsed)){
            return json::Node(get<int>(parsed));
    }
    return json::Node(get<double>(parsed));
}

std::string LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

json::Node String(istream& input) {
    return json::Node(move(LoadString(input)));
}

json::Node Bool(istream& input){
    std::string line;
    line = FindEndOfLine(input);
    line = ClearLine(line);
    if (line != "true" && line != "false") {
        throw ParsingError("Error while parsing JSON, wrong argument - bool");
    } else if (line == "false"){
        return json::Node(false);
    }
    return json::Node(true);
}

json::Node Dict(istream& input) {
    json::Dict result;
    if (input.peek() == -1) {
        throw ParsingError("Unexpected end of input");
    }
    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = load::String(input).AsString();
        input >> c;
        result.insert({move(key), load::Node(input)});
    }

    return json::Node(move(result));
}

json::Node Node(istream& input) {
    char c;
    while (input && 
        (c == '\n' || 
        c == '\r' || 
        c == '\t' || 
        c == ' ')) {
        input >> c;
    }
    input >> c;
    switch (c)
    {
    case 'n':
        input.putback(c);
        return load::Null(input);
        break;
    case 't':
        input.putback(c);
        return load::Bool(input);
        break;
    case 'f':
        input.putback(c);
        return load::Bool(input);
        break;
    case '[':
        return load::Array(input);
        break;
    case '{':
        return load::Dict(input);
        break;
    case '"':
        return load::String(input);
        break;
    default:
        if (isdigit(c) || c == '-') {
            input.putback(c);
            return load::Num(input);
        }
        else {
            std::cerr << "Unexpected input : '"<< c << "'"<< std::endl;
            throw ParsingError("Error while parsing JSON, wrong argument - node");
        }
        break;
    }
}

}  // load

//----------- Print ----------
namespace print {

    void Node (const json::Node& node, std::ostream& out, size_t tabulation);

    // Шаблон, подходящий для вывода double и int
    template <typename T>
    void Value(const T& value, std::ostream& out, [[maybe_unused]] size_t tabulation) {
        out << value;
    }

    // Перегрузка функции PrintValue для вывода значений null
    void Value(std::nullptr_t, std::ostream& out, [[maybe_unused]] size_t tabulation) {
        out << "null"sv;
    }

    // Перегрузка функции PrintValue для вывода значений bool
    void Value(bool value, std::ostream&out, [[maybe_unused]] size_t tabulation) {
        if (value) {
            out << "true"sv;
        } else {
            out << "false"sv;
        }
    }

    // Перегрузка функции PrintValue для вывода значений string
    void Value(const std::string& value, std::ostream& out, [[maybe_unused]] size_t tabulation){
        out << "\"";
        for (char c : value){
            switch (c)
            {
            case '\\':
                out << "\\\\"sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '\r':
                out << "\\r"sv;
                break;
            case '\"':
                out << "\\\""sv;
                break;
            case '\t':
                out << "\\t"sv;
                break;
            default:
                out << c;
                break;
            } 
        }
        out << "\"";
    }

    // Перегрузка функции PrintValue для вывода значений array
    void Value(const json::Array& array, std::ostream& out, size_t tabulation) {
        out << '[';
        if (array.empty()) {
            out << ']';
            return;
        }
        out <<  std::endl;
        tabulation += 4;
        bool first = true;
        for (auto& value : array){
            if (!first) {
                out << ',' << std::endl;
            }
            out << std::string(tabulation,' ');
            //if (value.IsBool()) out << std::endl;
            print::Node(value,out,tabulation);
            first = false;
        }
        out << std::endl << std::string(tabulation-4,' ') << ']';
    }

    // Перегрузка функции PrintValue для вывода значений dict
    void Value(const json::Dict& dict, std::ostream& out, size_t tabulation) {
        out << '{' << std::endl;
        tabulation += 4;
        bool first = true;
        for (const auto& [key,value] : dict){
            if (!first) {
                out << ',' << std::endl;
            }
            out << std::string(tabulation,' ') << '\"' << key << "\": ";
            print::Node(value,out,tabulation);
            first = false;
        }
        out << std::endl << std::string(tabulation-4,' ') << '}';
    }

    void Node(const json::Node& node, std::ostream& out, size_t tabulation) {
        std::visit(
            [&out, tabulation](const auto& value){ print::Value(value, out, tabulation); },
            node.GetValue());
    }

} // print


//----------- Node ----------


bool Node::IsInt() const {
    return std::holds_alternative<int>(node_);
}
bool Node::IsDouble() const{
    return (std::holds_alternative<double>(node_)||std::holds_alternative<int>(node_));
}
bool Node::IsPureDouble() const{
    return std::holds_alternative<double>(node_);
}
bool Node::IsBool() const{
    return std::holds_alternative<bool>(node_);
}
bool Node::IsString() const{
    return std::holds_alternative<std::string>(node_);
}
bool Node::IsNull() const{
    return std::holds_alternative<std::nullptr_t>(node_);
}
bool Node::IsArray() const{
    return std::holds_alternative<Array>(node_);
}
bool Node::IsMap() const{
    return std::holds_alternative<Dict>(node_);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error("Wrong value type");
    }
    return std::get<bool>(node_);
}
int Node::AsInt() const {
    if (!IsInt()) {
        throw std::logic_error("Wrong value type");
    }
    return std::get<int>(node_);
}
double Node::AsDouble() const {
    if (IsInt()) {
        return (static_cast<double>(AsInt()));
    }
    if (!IsDouble()) {
        throw std::logic_error("Wrong value type");
    }
    return std::get<double>(node_);
}
const string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("Wrong value type");
    }
    return std::get<std::string>(node_);
}
const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw std::logic_error("Wrong value type");
    }
    return std::get<Array>(node_);
}
const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw std::logic_error("Wrong value type");
    }
    return std::get<Dict>(node_);
}

//----------- Document ----------

Document::Document(Node root)
    : root_(move(root)) {
}

const json::Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{load::Node(input)};
}

void Print(const Document& doc, std::ostream& output) {
    output << std::setprecision(6);
    print::Node(doc.GetRoot(),output,0);
}

}  // namespace json