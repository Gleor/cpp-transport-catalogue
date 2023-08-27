#include "json_builder.h"

namespace json {

    using namespace std::literals;

    Builder::KeyItemContext Builder::Key(std::string key) {

        if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && !key_) {
            key_ = std::move(key);
            return KeyItemContext(*this);
        }

        throw std::logic_error("Can't insert key"s);
    }

    Builder& Builder::Value(Node::Value value) {

        if (root_.IsNull()) {
            root_.GetValue() = std::move(value);
            return *this;
        }

        if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && key_) {
            const_cast<Dict&>(nodes_stack_.back()->AsDict())[key_.value()] = std::move(value);
            key_ = std::nullopt;
            return *this;
        }

        if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
            const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(std::move(value));
            return *this;
        }
        throw std::logic_error("Can't insert value"s);
    }

    Builder::DictItemContext Builder::StartDict() {
        Value(Dict{});
        AddNodePtr(Node(Dict{}));
        return DictItemContext(*this);
    }

    Builder& Builder::EndDict() {
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && !key_) {
            nodes_stack_.pop_back();
            return *this;
        }
        throw std::logic_error("Can't close dict"s);
    }

    Builder::ArrayItemContext Builder::StartArray() {
        Value(Array{});
        AddNodePtr(Node(Array{}));
        return ArrayItemContext(*this);
    }

    Builder& Builder::EndArray() {
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
            nodes_stack_.pop_back();
            return *this;
        }
        throw std::logic_error("Can't close array"s);
    }

    const Node& Builder::Build() const {

        if (root_.IsNull() || !nodes_stack_.empty()) {
            throw std::logic_error("Build is invalid"s);
        }
        return root_;
    }

    void Builder::AddNodePtr(const Node& value) {
        if (value.IsDict() || value.IsArray()) {
            if (nodes_stack_.empty()) {
                nodes_stack_.push_back(&root_);
                return;
            }
            if (nodes_stack_.back()->IsDict()) {
                const json::Node* node = &nodes_stack_.back()->AsDict().at(key_.value());
                nodes_stack_.push_back(const_cast<Node*>(node));
                return;
            }
            if (nodes_stack_.back()->IsArray()) {
                const json::Node* node = &nodes_stack_.back()->AsArray().back();
                nodes_stack_.push_back(const_cast<Node*>(node));
                return;
            }
        }
    }

    Builder::ValueItemContext Builder::KeyItemContext::Value(Node::Value value) {
        builder_.Value(std::move(value));
        return ValueItemContext{ builder_ };
    }

    Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
        builder_.Value(std::move(value));
        return ArrayItemContext{ builder_ };
    }

    Builder::KeyItemContext Builder::ItemContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }

    Builder::DictItemContext Builder::ItemContext::StartDict() {
        return builder_.StartDict();
    }

    Builder& Builder::ItemContext::EndDict() {
        return builder_.EndDict();
    }

    Builder::ArrayItemContext Builder::ItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder& Builder::ItemContext::EndArray() {
        return builder_.EndArray();
    }

} // namespace json