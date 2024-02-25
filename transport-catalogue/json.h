#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {

    using Value = std::variant<std::nullptr_t,bool,int,double,std::string,Array,Dict>;

public:
    Node() = default;
    Node(std::nullptr_t np) : node_(np) {}
    Node(bool value) : node_(value){}
    Node(int value) : node_(value){}
    Node(double value) : node_(value){}
    Node(const std::string& value) : node_(std::move(value)) {}
    Node(const Array& array) : node_(std::move(array)) {}
    Node(const Dict& map) : node_(std::move(map)) {}


    bool IsInt() const;
	bool IsDouble() const;
	bool IsPureDouble() const;
	bool IsBool() const;
	bool IsString() const;
	bool IsNull() const;
	bool IsArray() const;
	bool IsMap() const;

	int AsInt() const;
	bool AsBool() const;
    //Возвращает значение типа double, если внутри хранится double либо int. 
    //В последнем случае возвращается приведённое в double значение.
	double AsDouble() const; 
	const std::string& AsString() const;
	const Array& AsArray() const;
	const Dict& AsMap() const;

    const Value& GetValue() const { 
        return node_; 
    } 

    bool operator==(const Node& other) const{
        return (node_ == other.node_);
    }
    bool operator!=(const Node& other) const{
        return !(*this == other);
    }

private:
    Value node_;
};


class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& other) const{
        return (root_ == other.root_);
    }
    bool operator!=(const Document& other) const{
        return !(*this == other);
    }

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json


