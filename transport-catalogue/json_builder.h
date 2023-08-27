#pragma once

#include <string>
#include <variant>
#include <vector>
#include <optional>

#include "json.h"

namespace json {

    class Builder final {

        class ItemContext;
        class KeyItemContext;
        class ValueItemContext;
        class ArrayItemContext;
        class DictItemContext;

    public:
        Builder() = default;

        const Node& Build() const;

        KeyItemContext Key(std::string key);
        Builder& Value(Node::Value value);

        ArrayItemContext StartArray();
        Builder& EndArray();

        DictItemContext StartDict();
        Builder& EndDict();

    private:
        Node root_{nullptr};

        std::vector<Node*> nodes_stack_;

        std::optional<std::string> key_{ std::nullopt };

        void AddNodePtr(const Node& value);
    };

    class Builder::ItemContext {
    public:
        ItemContext(Builder& builder)
            : builder_{ builder }
        {
        }
        KeyItemContext Key(std::string key);
        DictItemContext StartDict();
        Builder& EndDict();
        ArrayItemContext StartArray();
        Builder& EndArray();
        Builder& builder_;
    };

    class Builder::KeyItemContext final : private ItemContext {
    public:
        using ItemContext::ItemContext;
        ValueItemContext Value(Node::Value value);
        using ItemContext::StartDict;
        using ItemContext::StartArray;
    };

    class Builder::ValueItemContext final : private ItemContext {
    public:
        using ItemContext::ItemContext;
        using ItemContext::Key;
        using ItemContext::EndDict;
    };

    class Builder::ArrayItemContext final : private ItemContext {
    public:
        using ItemContext::ItemContext;
        ArrayItemContext Value(Node::Value value);
        using ItemContext::StartDict;
        using ItemContext::StartArray;
        using ItemContext::EndArray;
    };

    class Builder::DictItemContext final : private ItemContext {
    public:
        using ItemContext::ItemContext;
        using ItemContext::Key;
        using ItemContext::EndDict;
    };

} // namespace json