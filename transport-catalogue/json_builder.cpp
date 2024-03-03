#include "json_builder.h"

namespace json {


// ---------- Build ----------

    json::Node Builder::Build() {
        if(!object_is_ready){
            throw std::logic_error("Build() when object is not ready");
        }
        return root_;
    }

    json::Node Builder::BaseContext::Build() {
        return builder_.Build();
    }

// ---------- Key ----------

    Builder::DictValueContext Builder::Key(std::string key){
        if(object_is_ready){
            throw std::logic_error("Key() when object is ready");
        }
        if(!nodes_stack_.top()->IsMap()){
            throw std::logic_error("Key() when there is no Dict");
        }
        keys_.push(key);
        return DictValueContext(*this);
    }

    Builder::DictValueContext Builder::BaseContext::Key(std::string key){
        return builder_.Key(key);
    }

// ---------- Value ----------

    Builder::BaseContext Builder::Value(json::Node::Value value){
        if(object_is_ready){
            throw std::logic_error("Value() when object is ready");
        }
        if (!nodes_stack_.empty()){
            if (nodes_stack_.top()->IsArray()){
                AddValueToArray(value);
                return ArrayContext(*this);
            } else if (nodes_stack_.top()->IsMap()){
                AddValueToDict(value);
                return KeyItemContext(*this);
            } else {
                throw std::logic_error("Something went wrong!");
            }
        }
        root_ = Node(value);
        object_is_ready = true;
        return *this;
    }

    Builder::KeyItemContext Builder::DictValueContext::Value(json::Node::Value value){
        builder_.AddValueToDict(value);
        return KeyItemContext(*this);
    }

    Builder::ArrayContext Builder::ArrayContext::Value(json::Node::Value value){
        builder_.AddValueToArray(value);
        return ArrayContext(*this);
    }

// ---------- Start ----------

    Builder::KeyItemContext Builder::StartDict(){
        if(object_is_ready){
            throw std::logic_error("StartDict() when object is ready");
        }
        Dict new_dict;
        nodes_stack_.push(new Node(new_dict));
        return KeyItemContext(*this);
    }

    Builder::KeyItemContext Builder::BaseContext::StartDict(){
        return builder_.StartDict();
    }

    Builder::ArrayContext Builder::StartArray(){
        if(object_is_ready){
            throw std::logic_error("StartArray() when object is ready");
        }
        Array new_arr;
        nodes_stack_.push(new Node(new_arr));
        return ArrayContext(*this);
    }

    Builder::ArrayContext Builder::BaseContext::StartArray(){
        return builder_.StartArray();
    }

// ---------- End ----------

    Builder& Builder::EndDict(){
        if(object_is_ready){
            throw std::logic_error("EndDict() when object is ready");
        }
        if (nodes_stack_.empty()){
            throw std::logic_error("EndDict() when there is no Dict");
        }
        Dict this_dict = nodes_stack_.top()->AsMap();
         nodes_stack_.pop();
        if (!nodes_stack_.empty()){
            if ( nodes_stack_.top()->IsArray()){
                AddValueToArray(this_dict);
                return *this;
            } else if (nodes_stack_.top()->IsMap()){
                AddValueToDict(this_dict);
                return *this;
            } else {
                throw std::logic_error("Something went wrong!");
            }
        }
        root_ = Node(this_dict);
        object_is_ready = true;
        return *this;
    }

    Builder& Builder::BaseContext::EndDict(){
        return builder_.EndDict();
    }

    Builder& Builder::EndArray(){
        if(object_is_ready){
            throw std::logic_error("EndArray() when object is ready");
        }
        if (nodes_stack_.empty()){
            throw std::logic_error("EndArray() when there is no Array");
        }
        Array this_arr = nodes_stack_.top()->AsArray();
        nodes_stack_.pop();
        if (!nodes_stack_.empty()){
            if (nodes_stack_.top()->IsArray()){
                AddValueToArray(this_arr);
                return *this;
            } else if (nodes_stack_.top()->IsMap()){
                AddValueToDict(this_arr);
                return *this;
            } else {
                throw std::logic_error("Something went wrong!");
            }
        }
        root_ = Node(this_arr);
        object_is_ready = true;
        return *this;
    }

    Builder& Builder::BaseContext::EndArray(){
        return builder_.EndArray();
    }

// ---------- Utility ----------

    void Builder::AddValueToArray(Node::Value value){
        Array array = nodes_stack_.top()->AsArray();
        nodes_stack_.pop();
        array.push_back(Node(value));
        nodes_stack_.push(new Node(array));
    }

    void Builder::AddValueToDict(Node::Value value){
        if(keys_.empty()){
            throw std::logic_error("Key is not set");
        }
        Dict dict = nodes_stack_.top()->AsMap();
        nodes_stack_.pop();
        dict[keys_.top()] = Node(value);
        nodes_stack_.push(new Node(dict));
        keys_.pop();
    }
}