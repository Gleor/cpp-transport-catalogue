#include "json.h"

using namespace std;

namespace json {

    namespace {

        using Number = std::variant<int, double>;

        Node LoadNode(istream& input);

        Number NumericParser(std::istream& input) {

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
            }
            else {
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
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        std::string ProcessSpecialWords(std::istream& input) {
            std::string result;
            while (std::isalpha(input.peek()))
            {
                result.push_back(static_cast<char>(input.get()));
            }
            return result;
        }

        std::string StringParser(std::istream& input) {
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
                }
                else if (ch == '\\') {
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
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadNull(istream& input) {
            std::string check = ProcessSpecialWords(input);
            if (check == "null"sv) {
                return Node(nullptr);
            }
            else {
                throw ParsingError("Null parsing error");
            }

        }

        Node LoadBoolean(std::istream& input) {
            std::string check = ProcessSpecialWords(input);
            if (check == "true"sv) {
                return Node(true);
            }
            else if (check == "false"sv) {
                return Node(false);
            }
            else {
                throw ParsingError("Boolean parsing error");
            }
        }

        Node LoadNumber(istream& input) {
            Number result = NumericParser(input);
            if (std::holds_alternative<int>(result)) {
                return Node(std::get<int>(result));
            }
            else if (std::holds_alternative<double>(result)) {
                return Node(std::get<double>(result));
            }
            else {
                throw ParsingError("Number parsing error");
            }
        }

        Node LoadString(istream& input) {
            string line = StringParser(input);
            return Node(move(line));
        }

        Node LoadArray(istream& input) {
            Array result;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();

            if (it == end) {
                throw ParsingError("Array parsing error");
            }

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();

            if (it == end) {
                throw ParsingError("Dict parsing error");
            }

            for (char c; input >> c && c != '}';) {

                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }

            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            if (!(input >> c)) {
                throw ParsingError("Zero input_stream");
            }

            switch (c)
            {
            case '[':
                return LoadArray(input);

            case '{':
                return LoadDict(input);

            case '"':
                return LoadString(input);

            case 'n':
                input.putback(c);
                return LoadNull(input);

            case 't':
                input.putback(c);
                return LoadBoolean(input);

            case 'f':
                input.putback(c);
                return LoadBoolean(input);

            default:
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    Node::Node(Value value)
        : variant(std::move(value)) {
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return IsInt() || IsPureDouble();
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(*this);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("Not an Int");
        }
        return std::get<int>(*this);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("Not a Bool");
        }
        return std::get<bool>(*this);
    }

    double Node::AsDouble() const {
        if (!IsInt() && !IsPureDouble()) {
            throw std::logic_error("Not a Double");
        }
        else {
            double result_;
            if (IsInt()) {
                int int_result_ = std::get<int>(*this);
                result_ = int_result_;
            }
            else {
                result_ = std::get<double>(*this);
            }
            return result_;
        }
    }

    const std::string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("Not a String");
        }
        return std::get<std::string>(*this);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("Not an Array");
        }
        return std::get<Array>(*this);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw std::logic_error("Not a Map");
        }
        return std::get<Dict>(*this);
    }

    const Node::Value& Node::GetValue() const {
        return *this;
    }

    Node::Value& Node::GetValue() {
        return *this;
    }

    bool Node::operator==(const Node& rhs) const {
        return GetValue() == rhs.GetValue();
    }

    bool Node::operator!=(const Node& rhs) const {
        return !(GetValue() == rhs.GetValue());
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const Document& rhs) const {
        return root_ == rhs.root_;
    }

    bool Document::operator!=(const Document& rhs) const {
        return !(root_ == rhs.root_);
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void PrintContext::PrintIndent() const {
        for (int i = 0; i < _indent; ++i) {
            _out.put(' ');
        }
    }

    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        PrintValue(value, ctx);
    }

    void PrintValue(std::nullptr_t, const PrintContext& ctx) {
        ctx._out << "null"sv;
    }

    void PrintValue(const std::string& line, const PrintContext& ctx) {
        ctx._out.put('"');
        for (const char c : line) {
            switch (c)
            {
            case '\r':
                ctx._out << "\\r"sv;
                break;
            case '\n':
                ctx._out << "\\n"sv;
                break;
            case '"':
                // Символы " и \ выводятся как \" или \\, соответственно
                [[fallthrough]];
            case '\\':
                ctx._out.put('\\');
                [[fallthrough]];
            default:
                ctx._out.put(c);
                break;
            }
        }
        ctx._out.put('"');
    }

    void PrintValue(int num, const PrintContext& ctx) {
        ctx._out << num;
    }

    void PrintValue(double num, const PrintContext& ctx) {
        ctx._out << num;
    }

    void PrintValue(bool boolean, const PrintContext& ctx) {
        boolean ? ctx._out << "true"sv : ctx._out << "false"sv;
    }

    void PrintValue(const Array& array, const PrintContext& ctx) {
        auto& out = ctx._out;
        bool IsFirst = true;
        out << "[\n"sv;
        auto inner_ctx = ctx.Indented();
        for (auto& item : array) {
            if (!IsFirst) {
                out << ",\n"sv;
            }
            else {
                IsFirst = false;
            }
            inner_ctx.PrintIndent();
            std::visit(
                [&out, &item, &inner_ctx](const auto& value) {
                    PrintValue(value, inner_ctx); },
                    item.GetValue());
        }

        out.put('\n');
        ctx.PrintIndent();
        out.put(']');
    }

    void PrintValue(const Dict& dict, const PrintContext& ctx) {
        auto& out = ctx._out;
        bool IsFirst = true;
        out << "{\n"sv;
        auto inner_ctx = ctx.Indented();
        for (auto& item : dict) {
            if (!IsFirst) {

                out << ",\n"sv;
            }
            else {
                IsFirst = false;
            }
            inner_ctx.PrintIndent();
            PrintValue(item.first, inner_ctx);
            out << " : "sv;
            std::visit(
                [&out, &item, &inner_ctx](const auto& value) {
                    PrintValue(value, inner_ctx); },
                    item.second.GetValue());
        }

        out.put('\n');
        ctx.PrintIndent();
        out.put('}');
    }



    void Print(const Document& doc, std::ostream& output) {
        std::visit(
            [&output](const auto& value) {
                PrintValue(value, PrintContext(output, 4, 0)); }, doc.GetRoot().GetValue());
    }

}  // namespace json