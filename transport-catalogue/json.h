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
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final
        : Value {
    public:

        using variant::variant;
        using Value = variant;

        Node() = default;
        Node(Value value);

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        const std::string& AsString() const;
        bool AsBool() const;
        double AsDouble() const;

        const Value& GetValue() const;
        Value& GetValue();

        bool operator==(const Node& rhs) const;
        bool operator!=(const Node& rhs) const;

    private:
        Value value_ = nullptr;
    };

    class Document {
    public:
        Document() = default;

        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& other) const;
        bool operator!=(const Document& other) const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    struct PrintContext {
        PrintContext(std::ostream& out) : _out(out) {
        }

        PrintContext(std::ostream& out, int indent_step, int indent = 0)
            : _out(out)
            , _indent_step(indent_step)
            , _indent(indent) {
        }

        std::ostream& _out;
        int _indent_step = 4;
        int _indent = 0;

        void PrintIndent() const;

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return { _out, _indent_step, _indent_step + _indent };
        }

        PrintContext& GetContex() {
            return *this;
        }
    };
    void Print(const Document& doc, std::ostream& output);

}  // namespace json