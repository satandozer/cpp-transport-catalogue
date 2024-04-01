#pragma once

#include "json.h"
#include <stack>
#include <optional>

namespace json {

    class Builder {
            class BaseContext;
            class ArrayContext;
            class KeyItemContext;
            class DictItemContext;
            class DictValueContext;

            friend class BaseContext;
            friend class ArrayContext;
            friend class KeyItemContext;
            friend class DictItemContext;
            friend class DictValueContext;
        public:
        
            json::Node Build();
            BaseContext Value(json::Node::Value value);
            DictValueContext Key(std::string key);
            KeyItemContext StartDict();
            ArrayContext StartArray();
            Builder& EndDict();
            Builder& EndArray();  

        private:
            Node root_;
            std::stack<Node*> nodes_stack_;
            std::stack<std::string> keys_;
            bool object_is_ready = false;
            
            void AddValueToArray(Node::Value value);
            void AddValueToDict(Node::Value value);

            class BaseContext {
                public:
                    BaseContext(Builder& builder)
                        : builder_(builder){}

                    json::Node Build();
                    DictValueContext Key(std::string key);
                    KeyItemContext StartDict();
                    ArrayContext StartArray();
                    Builder& EndDict();
                    Builder& EndArray();                    
                protected:
                    Builder& builder_;
            };

            class KeyItemContext : public BaseContext {
                public:
                    KeyItemContext(BaseContext& context)
                        : BaseContext(context){}

                    KeyItemContext(Builder& builder)
                        : BaseContext(builder){}

                    json::Node Build() = delete;
                    KeyItemContext StartDict() = delete;
                    ArrayContext StartArray() = delete;
                    Builder& EndArray() = delete;
            };

            class DictItemContext : public BaseContext {
                public:
                    DictItemContext(BaseContext& context)
                        : BaseContext(context){}

                    DictItemContext(Builder& builder)
                        : BaseContext(builder){}

                    json::Node Build() = delete;
                    KeyItemContext StartDict() = delete;
                    ArrayContext StartArray() = delete;
                    Builder& EndArray() = delete; 
            };

            class DictValueContext : public BaseContext {
                public:
                    DictValueContext(BaseContext& context)
                        : BaseContext(context){}

                    DictValueContext(Builder& builder)
                        : BaseContext(builder){}

                    json::Node Build() = delete;
                    DictValueContext Key(std::string key) = delete;
                    Builder& EndArray() = delete;
                    Builder& EndDict() = delete;
                    KeyItemContext Value(Node::Value value);
            };

            class ArrayContext : public BaseContext {
                public:
                    ArrayContext(BaseContext& context)
                        : BaseContext(context){}

                    ArrayContext(Builder& builder)
                        : BaseContext(builder){}


                    json::Node Build() = delete;
                    DictValueContext Key(std::string key) = delete;
                    Builder& EndDict() = delete;

                    ArrayContext Value(Node::Value value);
            };
            
    };


}