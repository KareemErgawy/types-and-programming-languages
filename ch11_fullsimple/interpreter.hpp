#pragma once

#include <algorithm>
#include <cassert>
#include <deque>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace lexer {
struct Token {
    enum class Category {
        IDENTIFIER,

        LAMBDA,

        DOT,
        COMMA,
        EQUAL,
        OPEN_PAREN,
        CLOSE_PAREN,
        OPEN_BRACE,
        CLOSE_BRACE,
        COLON,
        ARROW,

        CONSTANT_TRUE,
        CONSTANT_FALSE,

        KEYWORD_BOOL,
        KEYWORD_IF,
        KEYWORD_THEN,
        KEYWORD_ELSE,

        CONSTANT_ZERO,

        KEYWORD_NAT,
        KEYWORD_SUCC,
        KEYWORD_PRED,
        KEYWORD_ISZERO,

        MARKER_END,
        MARKER_INVALID
    };

    Token(Category category = Category::MARKER_INVALID, std::string text = "")
        : category_(category),
          text_(category == Category::IDENTIFIER ? text : "") {}

    bool operator==(const Token& other) const {
        return category_ == other.category_ && text_ == other.text_;
    }

    bool operator!=(const Token& other) const { return !(*this == other); }

    Category GetCategory() const { return category_; }

    std::string GetText() const { return text_; }

   private:
    Category category_ = Category::MARKER_INVALID;
    std::string text_;
};

std::ostream& operator<<(std::ostream& out, Token token);

namespace {
const std::string kLambdaInputSymbol = "l";
const std::string kKeywordBool = "Bool";
const std::string kKeywordNat = "Nat";
}  // namespace

class Lexer {
   public:
    Lexer(std::istringstream&& in) {
        std::istringstream iss(SurroundTokensBySpaces(std::move(in)));
        token_strings_ =
            std::vector<std::string>(std::istream_iterator<std::string>{iss},
                                     std::istream_iterator<std::string>());
    }

    Token NextToken() {
        Token token;

        if (current_token_ == token_strings_.size()) {
            token = Token(Token::Category::MARKER_END);
            return token;
        }

        std::unordered_map<std::string, Token::Category> token_str_to_cat = {
            {kLambdaInputSymbol, Token::Category::LAMBDA},

            {".", Token::Category::DOT},
            {",", Token::Category::COMMA},
            {"=", Token::Category::EQUAL},
            {"(", Token::Category::OPEN_PAREN},
            {")", Token::Category::CLOSE_PAREN},
            {"{", Token::Category::OPEN_BRACE},
            {"}", Token::Category::CLOSE_BRACE},
            {":", Token::Category::COLON},
            {"->", Token::Category::ARROW},

            {"true", Token::Category::CONSTANT_TRUE},
            {"false", Token::Category::CONSTANT_FALSE},

            {kKeywordBool, Token::Category::KEYWORD_BOOL},
            {"if", Token::Category::KEYWORD_IF},
            {"then", Token::Category::KEYWORD_THEN},
            {"else", Token::Category::KEYWORD_ELSE},

            {"0", Token::Category::CONSTANT_ZERO},

            {kKeywordNat, Token::Category::KEYWORD_NAT},
            {"succ", Token::Category::KEYWORD_SUCC},
            {"pred", Token::Category::KEYWORD_PRED},
            {"iszero", Token::Category::KEYWORD_ISZERO},
        };

        auto token_string = token_strings_[current_token_];

        if (token_str_to_cat.find(token_string) != std::end(token_str_to_cat)) {
            token = Token(token_str_to_cat[token_string]);
        } else if (IsIdentifierName(token_string)) {
            token = Token(Token::Category::IDENTIFIER, token_string);
        }

        ++current_token_;

        return token;
    }

    void PutBackToken() {
        if (current_token_ > 0) {
            --current_token_;
        }
    }

   private:
    std::string SurroundTokensBySpaces(std::istringstream&& in) {
        std::ostringstream processed_stream;
        char c;

        while (in.get(c)) {
            // Check for one-character separators and surround them with spaces.
            if (c == ':' || c == ',' || c == '.' || c == '=' || c == '(' ||
                c == ')' || c == '{' || c == '}') {
                processed_stream << " " << c << " ";
            } else if (c == '-') {
                // Check for the only two-character serparator '->' and surround
                // it with spaces.
                if (in.peek() == '>') {
                    in.get(c);
                    processed_stream << " -> ";
                } else {
                    // Just write '-' and let the lexing error be
                    // discovered later.
                    processed_stream << " - ";
                }
            } else {
                processed_stream << c;
            }
        }

        return processed_stream.str();
    }

    bool IsIdentifierName(std::string token_text) {
        for (auto c : token_text) {
            if (!std::isalpha(c) && c != '_') {
                return false;
            }
        }

        return !token_text.empty();
    }

   private:
    std::vector<std::string> token_strings_;
    int current_token_ = 0;
};

std::ostream& operator<<(std::ostream& out, Token token) {
    std::unordered_map<Token::Category, std::string> token_to_str = {
        {Token::Category::LAMBDA, "λ"},

        {Token::Category::DOT, "."},
        {Token::Category::COMMA, ","},
        {Token::Category::EQUAL, "="},
        {Token::Category::OPEN_PAREN, "("},
        {Token::Category::CLOSE_PAREN, ")"},
        {Token::Category::OPEN_BRACE, "{"},
        {Token::Category::CLOSE_BRACE, "}"},
        {Token::Category::COLON, ":"},
        {Token::Category::ARROW, "->"},

        {Token::Category::CONSTANT_TRUE, "<true>"},
        {Token::Category::CONSTANT_FALSE, "<false>"},

        {Token::Category::KEYWORD_BOOL, "<Bool>"},
        {Token::Category::KEYWORD_IF, "<if>"},
        {Token::Category::KEYWORD_THEN, "<then>"},
        {Token::Category::KEYWORD_ELSE, "<else>"},

        {Token::Category::CONSTANT_ZERO, "0"},

        {Token::Category::KEYWORD_NAT, "<Nat>"},
        {Token::Category::KEYWORD_SUCC, "succ"},
        {Token::Category::KEYWORD_PRED, "pred"},
        {Token::Category::KEYWORD_ISZERO, "iszero"},

        {Token::Category::MARKER_END, "<END>"},
        {Token::Category::MARKER_INVALID, "<INVALID>"},
    };

    if (token_to_str.find(token.GetCategory()) != std::end(token_to_str)) {
        out << token_to_str[token.GetCategory()];
    } else {
        switch (token.GetCategory()) {
            case Token::Category::IDENTIFIER:
                out << token.GetText();
                break;

            default:
                out << "<ILLEGAL_TOKEN>";
        }
    }

    return out;
}
}  // namespace lexer

namespace parser {

class Type {
    friend std::ostream& operator<<(std::ostream&, const Type&);

   public:
    static Type& IllTyped() {
        static Type type;

        return type;
    }

    static Type& Bool() {
        static Type type(BaseType::BOOL);

        return type;
    }

    static Type& Nat() {
        static Type type(BaseType::NAT);

        return type;
    }

    static Type& Function(Type& lhs, Type& rhs) {
        static std::vector<std::unique_ptr<Type>> type_pool;

        auto result =
            std::find_if(std::begin(type_pool), std::end(type_pool),
                         [&](const std::unique_ptr<Type>& type) {
                             return type->lhs_ == &lhs && type->rhs_ == &rhs;
                         });

        if (result != std::end(type_pool)) {
            return **result;
        }

        type_pool.emplace_back(std::unique_ptr<Type>(new Type(lhs, rhs)));

        return *type_pool.back();
    }

    using RecordFields = std::vector<std::pair<std::string, Type&>>;

    static Type& Record(RecordFields fields) {
        static std::vector<std::unique_ptr<Type>> type_pool;

        auto result = std::find_if(std::begin(type_pool), std::end(type_pool),
                                   [&](const std::unique_ptr<Type>& type) {
                                       return type->record_fields_ == fields;
                                   });

        if (result != std::end(type_pool)) {
            return **result;
        }

        type_pool.emplace_back(
            std::unique_ptr<Type>(new Type(std::move(fields))));

        return *type_pool.back();
    }

    Type(const Type&) = delete;
    Type& operator=(const Type&) = delete;

    Type(Type&&) = delete;
    Type& operator=(Type&&) = delete;

    ~Type() = default;

    bool operator==(const Type& other) const {
        if (category_ != other.category_) {
            return false;
        }

        switch (category_) {
            case TypeCategory::ILL:
                return other.category_ == TypeCategory::ILL;
            case TypeCategory::BASE:
                return base_type_ == other.base_type_;
            case TypeCategory::FUNCTION:
                assert(lhs_ && rhs_ && other.lhs_ && other.rhs_);
                return (*lhs_ == *other.lhs_) && (*rhs_ == *other.rhs_);
            case TypeCategory::RECORD:
                return record_fields_ == other.record_fields_;
        }
    }

    bool operator!=(const Type& other) const { return !(*this == other); }

    bool IsIllTyped() const { return category_ == TypeCategory::BASE; }

    bool IsBool() const {
        return category_ == TypeCategory::BASE && base_type_ == BaseType::BOOL;
    }

    bool IsNat() const {
        return category_ == TypeCategory::BASE && base_type_ == BaseType::NAT;
    }

    bool IsFunction() const { return category_ == TypeCategory::FUNCTION; }

    bool IsRecord() const { return category_ == TypeCategory::RECORD; }

    Type& FunctionLHS() const {
        if (!IsFunction()) {
            throw std::invalid_argument("Invalid function type.");
        }

        return *lhs_;
    }

    Type& FunctionRHS() const {
        if (!IsFunction()) {
            throw std::invalid_argument("Invalid function type.");
        }

        return *rhs_;
    }

   private:
    Type(Type& lhs, Type& rhs)
        : lhs_(&lhs), rhs_(&rhs), category_(TypeCategory::FUNCTION) {}

    Type(RecordFields fields)
        : record_fields_(std::move(fields)), category_(TypeCategory::RECORD) {}

    enum class TypeCategory {
        BASE,
        FUNCTION,
        RECORD,
        ILL,
    };

    enum class BaseType {
        BOOL,
        NAT,
    };

    Type() = default;

    Type(BaseType base_type)
        : base_type_(base_type), category_(TypeCategory::BASE) {}

    TypeCategory category_ = TypeCategory::ILL;

    BaseType base_type_;

    Type* lhs_ = nullptr;
    Type* rhs_ = nullptr;

    RecordFields record_fields_{};
};

std::ostream& operator<<(std::ostream& out, const Type& type) {
    if (type.IsBool()) {
        out << lexer::kKeywordBool;
    } else if (type.IsNat()) {
        out << lexer::kKeywordNat;
    } else if (type.IsFunction()) {
        out << "(" << *type.lhs_ << " "
            << lexer::Token(lexer::Token::Category::ARROW) << " " << *type.rhs_
            << ")";
    } else if (type.IsRecord()) {
        out << "{";

        for (int i = 0; i < type.record_fields_.size(); ++i) {
            if (i > 0) {
                out << ", ";
            }

            out << type.record_fields_[i].first << ":"
                << type.record_fields_[i].second;
        }

        out << "}";
    } else {
        out << "Ⱦ";
    }

    return out;
}

class Term {
    friend std::ostream& operator<<(std::ostream&, const Term&);

   public:
    static Term Lambda(std::string arg_name, Type& arg_type) {
        Term result;
        result.lambda_arg_name_ = arg_name;
        result.lambda_arg_type_ = &arg_type;
        result.is_lambda_ = true;

        return result;
    }

    static Term Variable(std::string var_name, int de_bruijn_idx) {
        Term result;
        result.variable_name_ = var_name;
        result.de_bruijn_idx_ = de_bruijn_idx;
        result.is_variable_ = true;

        return result;
    }

    static Term Application(std::unique_ptr<Term> lhs,
                            std::unique_ptr<Term> rhs) {
        Term result;
        result.is_application_ = true;
        result.application_lhs_ = std::move(lhs);
        result.application_rhs_ = std::move(rhs);

        return result;
    }

    static Term If() {
        Term result;
        result.is_if_ = true;

        return result;
    }

    static Term True() {
        Term result;
        result.is_true_ = true;

        return result;
    }

    static Term False() {
        Term result;
        result.is_false_ = true;

        return result;
    }

    static Term Succ() {
        Term result;
        result.is_succ_ = true;

        return result;
    }

    static Term Pred() {
        Term result;
        result.is_pred_ = true;

        return result;
    }

    static Term IsZero() {
        Term result;
        result.is_iszero_ = true;

        return result;
    }

    static Term Zero() {
        Term result;
        result.is_zero_ = true;

        return result;
    }

    Term() = default;

    Term(const Term&) = delete;
    Term& operator=(const Term&) = delete;

    Term(Term&&) = default;
    Term& operator=(Term&&) = default;

    ~Term() = default;

    bool IsLambda() const { return is_lambda_; }

    void MarkLambdaAsComplete() { is_complete_lambda_ = true; }

    bool IsVariable() const { return is_variable_; }

    bool IsApplication() const { return is_application_; }

    bool IsIf() const { return is_if_; }

    bool IsTrue() const { return is_true_; }

    bool IsFalse() const { return is_false_; }

    bool IsSucc() const { return is_succ_; }

    bool IsPred() const { return is_pred_; }

    bool IsIsZero() const { return is_iszero_; }

    bool IsConstantZero() const { return is_zero_; }

    bool IsInvalid() const {
        if (IsLambda()) {
            return lambda_arg_name_.empty() || !lambda_arg_type_ ||
                   !lambda_body_;
        } else if (IsVariable()) {
            return variable_name_.empty();
        } else if (IsApplication()) {
            return !application_lhs_ || !application_rhs_;
        } else if (IsIf()) {
            return !if_condition_ || !if_then_ || !if_else_;
        } else if (IsTrue() || IsFalse() || IsConstantZero()) {
            return false;
        } else if (IsSucc()) {
            return !unary_op_arg_;
        } else if (IsPred()) {
            return !unary_op_arg_;
        } else if (IsIsZero()) {
            return !unary_op_arg_;
        }

        return true;
    }

    bool IsEmpty() const {
        return !IsLambda() && !IsVariable() && !IsApplication() && !IsIf() &&
               !IsSucc() && !IsPred() && !IsIsZero();
    }

    Term& Combine(Term&& term) {
        if (term.IsInvalid()) {
            throw std::invalid_argument(
                "Term::Combine() received an invalid Term.");
        }

        if (IsLambda()) {
            if (lambda_body_) {
                // If the lambda body was completely parsed, then combining this
                // term and the argument term means applying this lambda to the
                // argument.
                if (is_complete_lambda_) {
                    *this =
                        Application(std::make_unique<Term>(std::move(*this)),
                                    std::make_unique<Term>(std::move(term)));

                    is_lambda_ = false;
                    lambda_body_ = nullptr;
                    lambda_arg_name_ = "";
                    is_complete_lambda_ = false;
                } else {
                    lambda_body_->Combine(std::move(term));
                }
            } else {
                lambda_body_ = std::make_unique<Term>(std::move(term));
            }
        } else if (IsVariable()) {
            *this = Application(std::make_unique<Term>(std::move(*this)),
                                std::make_unique<Term>(std::move(term)));

            is_variable_ = false;
            variable_name_ = "";
        } else if (IsApplication()) {
            *this = Application(std::make_unique<Term>(std::move(*this)),
                                std::make_unique<Term>(std::move(term)));
        } else if (IsIf()) {
            if (!if_condition_) {
                if_condition_ = std::make_unique<Term>(std::move(term));
            } else if (!if_then_) {
                if_then_ = std::make_unique<Term>(std::move(term));
            } else {
                if (!if_else_) {
                    if_else_ = std::make_unique<Term>(std::move(term));
                } else {
                    // If the if condition was completely parsed, then combining
                    // this term and the argument term means applying this
                    // lambda to the argument.
                    *this =
                        Application(std::make_unique<Term>(std::move(*this)),
                                    std::make_unique<Term>(std::move(term)));

                    is_if_ = false;
                    if_condition_ = nullptr;
                    if_then_ = nullptr;
                    if_else_ = nullptr;
                }
            }
        } else if (IsSucc()) {
            if (!unary_op_arg_) {
                unary_op_arg_ = std::make_unique<Term>(std::move(term));
            } else {
                throw std::invalid_argument(
                    "Trying to combine with succ(...).");
            }
        } else if (IsPred()) {
            if (!unary_op_arg_) {
                unary_op_arg_ = std::make_unique<Term>(std::move(term));
            } else {
                throw std::invalid_argument(
                    "Trying to combine with pred(...).");
            }
        } else if (IsIsZero()) {
            if (!unary_op_arg_) {
                unary_op_arg_ = std::make_unique<Term>(std::move(term));
            } else {
                throw std::invalid_argument(
                    "Trying to combine with iszero(...).");
            }
        } else if (IsTrue() || IsFalse() || IsConstantZero()) {
            throw std::invalid_argument("Trying to combine with a constant.");
        } else {
            *this = std::move(term);
        }

        return *this;
    }

    /*
     * Shifts the de Bruijn indices of all free variables inside this Term up by
     * distance amount. For an example use, see Term::Substitute(int, Term&).
     */
    void Shift(int distance) {
        std::function<void(int, Term&)> walk = [&distance, &walk](
                                                   int binding_context_size,
                                                   Term& term) {
            if (term.IsInvalid()) {
                throw std::invalid_argument("Trying to shift an invalid term.");
            }

            if (term.IsVariable()) {
                if (term.de_bruijn_idx_ >= binding_context_size) {
                    term.de_bruijn_idx_ += distance;
                }
            } else if (term.IsLambda()) {
                walk(binding_context_size + 1, *term.lambda_body_);
            } else if (term.IsApplication()) {
                walk(binding_context_size, *term.application_lhs_);
                walk(binding_context_size, *term.application_rhs_);
            }
        };

        walk(0, *this);
    }

    /**
     * Substitutes variable (that is, the de Brijun idex of a variable) with the
     * term sub.
     */
    void Substitute(int variable, Term& sub) {
        if (IsInvalid() || sub.IsInvalid()) {
            throw std::invalid_argument(
                "Trying to substitute using invalid terms.");
        }

        std::function<void(int, Term&)> walk = [&variable, &sub, &walk](
                                                   int binding_context_size,
                                                   Term& term) {
            if (term.IsVariable()) {
                // Adjust variable according to the current binding
                // depth before comparing term's index.
                if (term.de_bruijn_idx_ == variable + binding_context_size) {
                    // Shift sub up by binding_context_size distance since sub
                    // is now substituted in binding_context_size deep context.
                    auto clone = sub.Clone();
                    clone.Shift(binding_context_size);
                    std::swap(term, clone);
                }
            } else if (term.IsLambda()) {
                walk(binding_context_size + 1, *term.lambda_body_);
            } else if (term.IsApplication()) {
                walk(binding_context_size, *term.application_lhs_);
                walk(binding_context_size, *term.application_rhs_);
            } else if (term.IsIf()) {
                walk(binding_context_size, *term.if_condition_);
                walk(binding_context_size, *term.if_then_);
                walk(binding_context_size, *term.if_else_);
            } else if (term.IsSucc()) {
                walk(binding_context_size, *term.unary_op_arg_);
            } else if (term.IsPred()) {
                walk(binding_context_size, *term.unary_op_arg_);
            } else if (term.IsIsZero()) {
                walk(binding_context_size, *term.unary_op_arg_);
            }
        };

        walk(0, *this);
    }

    Term& LambdaBody() const {
        if (!IsLambda()) {
            throw std::invalid_argument("Invalid Lambda term.");
        }

        return *lambda_body_;
    }

    std::string LambdaArgName() const {
        if (!IsLambda()) {
            throw std::invalid_argument("Invalid Lambda term.");
        }

        return lambda_arg_name_;
    }

    Type& LambdaArgType() const {
        if (!IsLambda()) {
            throw std::invalid_argument("Invalid Lambda term.");
        }

        return *lambda_arg_type_;
    }

    std::string VariableName() const {
        if (!IsVariable()) {
            throw std::invalid_argument("Invalid variable term.");
        }

        return variable_name_;
    }

    int VariableDeBruijnIdx() const {
        if (!IsVariable()) {
            throw std::invalid_argument("Invalid variable term.");
        }

        return de_bruijn_idx_;
    }

    Term& ApplicationLHS() const {
        if (!IsApplication()) {
            throw std::invalid_argument("Invalid application term.");
        }

        return *application_lhs_;
    }

    Term& ApplicationRHS() const {
        if (!IsApplication()) {
            throw std::invalid_argument("Invalid application term.");
        }

        return *application_rhs_;
    }

    Term& IfCondition() const {
        if (!IsIf()) {
            throw std::invalid_argument("Invalid if term.");
        }

        return *if_condition_;
    }

    Term& IfThen() const {
        if (!IsIf()) {
            throw std::invalid_argument("Invalid if term.");
        }

        return *if_then_;
    }

    Term& IfElse() const {
        if (!IsIf()) {
            throw std::invalid_argument("Invalid if term.");
        }

        return *if_else_;
    }

    Term& UnaryOpArg() const {
        if (!IsSucc() && !IsPred() && !IsIsZero()) {
            throw std::invalid_argument("Invalid term.");
        }

        return *unary_op_arg_;
    }

    bool operator==(const Term& other) const {
        if (IsLambda() && other.IsLambda()) {
            return LambdaArgType() == other.LambdaArgType() &&
                   LambdaBody() == other.LambdaBody();
        }

        if (IsVariable() && other.IsVariable()) {
            return de_bruijn_idx_ == other.de_bruijn_idx_;
        }

        if (IsApplication() && other.IsApplication()) {
            return (ApplicationLHS() == other.ApplicationLHS()) &&
                   (ApplicationRHS() == other.ApplicationRHS());
        }

        if (IsIf() && other.IsIf()) {
            return (IfCondition() == other.IfCondition()) &&
                   (IfThen() == other.IfThen()) && (IfElse() == other.IfElse());
        }

        if (IsTrue() && other.IsTrue()) {
            return true;
        }

        if (IsFalse() && other.IsFalse()) {
            return true;
        }

        if (IsSucc() && other.IsSucc()) {
            return UnaryOpArg() == other.UnaryOpArg();
        }

        if (IsPred() && other.IsPred()) {
            return UnaryOpArg() == other.UnaryOpArg();
        }

        if (IsIsZero() && other.IsIsZero()) {
            return UnaryOpArg() == other.UnaryOpArg();
        }

        if (IsConstantZero() && other.IsConstantZero()) {
            return true;
        }

        return false;
    }

    bool operator!=(const Term& other) const { return !(*this == other); }

    std::string ASTString(int indentation = 0) const {
        std::ostringstream out;
        std::string prefix = std::string(indentation, '-');

        if (IsLambda()) {
            out << prefix << "λ " << lambda_arg_name_ << ":"
                << *lambda_arg_type_ << "\n";
            out << lambda_body_->ASTString(indentation + 2);
        } else if (IsVariable()) {
            out << prefix << variable_name_ << "[" << de_bruijn_idx_ << "]";
        } else if (IsApplication()) {
            out << prefix << "<-\n";
            out << application_lhs_->ASTString(indentation + 2) << "\n";
            out << application_rhs_->ASTString(indentation + 2);
        } else if (IsIf()) {
            out << prefix << "if\n";
            out << if_condition_->ASTString(indentation + 2) << "\n";
            out << prefix << "then\n";
            out << if_then_->ASTString(indentation + 2) << "\n";
            out << prefix << "else\n";
            out << if_else_->ASTString(indentation + 2);
        } else if (IsTrue()) {
            out << prefix << "true";
        } else if (IsFalse()) {
            out << prefix << "false";
        } else if (IsSucc()) {
            out << prefix << "succ\n";
            out << unary_op_arg_->ASTString(indentation + 2);
        } else if (IsPred()) {
            out << prefix << "pred\n";
            out << unary_op_arg_->ASTString(indentation + 2);
        } else if (IsIsZero()) {
            out << prefix << "iszero\n";
            out << unary_op_arg_->ASTString(indentation + 2);
        } else if (IsConstantZero()) {
            out << prefix << "0";
        }

        return out.str();
    }

    Term Clone() const {
        if (IsInvalid()) {
            throw std::logic_error("Trying to clone an invalid term.");
        }

        if (IsLambda()) {
            return std::move(Lambda(lambda_arg_name_, *lambda_arg_type_)
                                 .Combine(lambda_body_->Clone()));
        } else if (IsVariable()) {
            return Variable(variable_name_, de_bruijn_idx_);
        } else if (IsApplication()) {
            return Application(
                std::make_unique<Term>(application_lhs_->Clone()),
                std::make_unique<Term>(application_rhs_->Clone()));
        } else if (IsTrue()) {
            return Term::True();
        } else if (IsFalse()) {
            return Term::False();
        } else if (IsConstantZero()) {
            return Term::Zero();
        } else if (IsSucc()) {
            return std::move(Term::Succ().Combine(unary_op_arg_->Clone()));
        } else if (IsSucc()) {
            return std::move(Term::Pred().Combine(unary_op_arg_->Clone()));
        } else if (IsIsZero()) {
            return std::move(Term::IsZero().Combine(unary_op_arg_->Clone()));
        }

        std::ostringstream error_ss;
        error_ss << "Couldn't clone term: " << *this;
        throw std::logic_error(error_ss.str());
    }

    bool is_complete_lambda_ = false;

   private:
    bool is_lambda_ = false;
    std::string lambda_arg_name_ = "";
    Type* lambda_arg_type_ = nullptr;
    std::unique_ptr<Term> lambda_body_{};
    // Marks whether parsing for the body of the lambda term is finished or not.

    bool is_variable_ = false;
    std::string variable_name_ = "";
    int de_bruijn_idx_ = -1;

    bool is_application_ = false;
    std::unique_ptr<Term> application_lhs_{};
    std::unique_ptr<Term> application_rhs_{};

    bool is_if_ = false;
    std::unique_ptr<Term> if_condition_{};
    std::unique_ptr<Term> if_then_{};
    std::unique_ptr<Term> if_else_{};

    bool is_true_ = false;

    bool is_false_ = false;

    bool is_succ_ = false;
    bool is_pred_ = false;
    bool is_iszero_ = false;
    std::unique_ptr<Term> unary_op_arg_{};

    bool is_zero_ = false;
};

std::ostream& operator<<(std::ostream& out, const Term& term) {
    if (term.IsInvalid()) {
        out << "<INVALID>";
    } else if (term.IsVariable()) {
        out << term.variable_name_;
    } else if (term.IsLambda()) {
        out << "{l " << term.lambda_arg_name_ << " : " << *term.lambda_arg_type_
            << ". " << *term.lambda_body_ << "}";
    } else if (term.IsApplication()) {
        out << "(" << *term.application_lhs_ << " <- " << *term.application_rhs_
            << ")";
    } else if (term.IsIf()) {
        out << "if (" << *term.if_condition_ << ") then (" << *term.if_then_
            << ") else (" << *term.if_else_ << ")";
    } else if (term.IsTrue()) {
        out << "true";
    } else if (term.IsFalse()) {
        out << "false";
    } else if (term.IsSucc()) {
        out << "succ (" << *term.unary_op_arg_ << ")";
    } else if (term.IsPred()) {
        out << "succ (" << *term.unary_op_arg_ << ")";
    } else if (term.IsIsZero()) {
        out << "succ (" << *term.unary_op_arg_ << ")";
    } else if (term.IsConstantZero()) {
        out << "0";
    } else {
        out << "<ERROR>";
    }

    return out;
}

class Parser {
    using Token = lexer::Token;

   public:
    Parser(std::istringstream&& in) : lexer_(std::move(in)) {}

    Term ParseProgram() {
        Token next_token;
        std::vector<Term> term_stack;
        term_stack.emplace_back(Term());
        int balance_parens = 0;
        // For each '(', records the size of term_stack when the '(' was parsed.
        // This is used later when the corresponding ')' is parsed to know how
        // many Terms from term_stack should be popped (i.e. their parsing is
        // know to be complete).
        std::vector<int> stack_size_on_open_paren;
        // Contains a list of bound variables in order of binding. For example,
        // for a term λ x. λ y. x y, this list would eventually contains {"x" ,
        // "y"} in that order. This is used to assign de Bruijn indices/static
        // distances to bound variables (ref: tapl,§6.1).
        std::vector<std::string> bound_variables;

        while ((next_token = lexer_.NextToken()).GetCategory() !=
               Token::Category::MARKER_END) {
            if (next_token.GetCategory() == Token::Category::LAMBDA) {
                auto lambda_arg = ParseLambdaArg();
                auto lambda_arg_name = lambda_arg.first;
                bound_variables.push_back(lambda_arg_name);

                // If the current stack top is empty, use its slot for the
                // lambda.
                if (term_stack.back().IsEmpty()) {
                    term_stack.back() =
                        Term::Lambda(lambda_arg_name, lambda_arg.second);
                } else {
                    // Else, push a new term on the stack to start building the
                    // lambda term.
                    term_stack.emplace_back(
                        Term::Lambda(lambda_arg_name, lambda_arg.second));
                }
            } else if (next_token.GetCategory() ==
                       Token::Category::IDENTIFIER) {
                auto bound_variable_it =
                    std::find(std::begin(bound_variables),
                              std::end(bound_variables), next_token.GetText());
                int de_bruijn_idx = -1;

                if (bound_variable_it != std::end(bound_variables)) {
                    de_bruijn_idx = std::distance(bound_variable_it,
                                                  std::end(bound_variables)) -
                                    1;
                } else {
                    // The naming context for free variables (ref: tapl,§6.1.2)
                    // is chosen to be the ASCII code of a variable's name.
                    //
                    // NOTE: Only single-character variable names are currecntly
                    // supported as free variables.
                    if (next_token.GetText().length() != 1) {
                        std::ostringstream error_ss;
                        error_ss << "Unexpected token: " << next_token;
                        throw std::invalid_argument(error_ss.str());
                    }

                    de_bruijn_idx =
                        bound_variables.size() +
                        (std::tolower(next_token.GetText()[0]) - 'a');
                }

                term_stack.back().Combine(
                    Term::Variable(next_token.GetText(), de_bruijn_idx));
            } else if (next_token.GetCategory() ==
                       Token::Category::KEYWORD_IF) {
                // If the current stack top is empty, use its slot for the
                // if condition.
                if (term_stack.back().IsEmpty()) {
                    term_stack.back() = Term::If();
                } else {
                    // Else, push a new term on the stack to start building the
                    // if condition.
                    term_stack.emplace_back(Term::If());
                }

                stack_size_on_open_paren.emplace_back(term_stack.size());
                term_stack.emplace_back(Term());
                ++balance_parens;
            } else if (next_token.GetCategory() ==
                       Token::Category::KEYWORD_THEN) {
                UnwindStack(term_stack, stack_size_on_open_paren,
                            bound_variables);

                --balance_parens;

                if (!term_stack.back().IsIf()) {
                    throw std::invalid_argument("Unexpected 'then'");
                }

                stack_size_on_open_paren.emplace_back(term_stack.size());
                term_stack.emplace_back(Term());
                ++balance_parens;
            } else if (next_token.GetCategory() ==
                       Token::Category::KEYWORD_ELSE) {
                UnwindStack(term_stack, stack_size_on_open_paren,
                            bound_variables);

                --balance_parens;

                if (!term_stack.back().IsIf()) {
                    throw std::invalid_argument("Unexpected 'else'");
                }
            } else if (next_token.GetCategory() ==
                       Token::Category::KEYWORD_SUCC) {
                // If the current stack top is empty, use its slot for the
                // succ term.
                if (term_stack.back().IsEmpty()) {
                    term_stack.back() = Term::Succ();
                } else {
                    // Else, push a new term on the stack to start building the
                    // if condition.
                    term_stack.emplace_back(Term::Succ());
                }
            } else if (next_token.GetCategory() ==
                       Token::Category::KEYWORD_PRED) {
                // If the current stack top is empty, use its slot for the
                // pred term.
                if (term_stack.back().IsEmpty()) {
                    term_stack.back() = Term::Pred();
                } else {
                    // Else, push a new term on the stack to start building the
                    // if condition.
                    term_stack.emplace_back(Term::Pred());
                }
            } else if (next_token.GetCategory() ==
                       Token::Category::KEYWORD_ISZERO) {
                // If the current stack top is empty, use its slot for the
                // iszero term.
                if (term_stack.back().IsEmpty()) {
                    term_stack.back() = Term::IsZero();
                } else {
                    // Else, push a new term on the stack to start building the
                    // if condition.
                    term_stack.emplace_back(Term::IsZero());
                }
            } else if (next_token.GetCategory() ==
                       Token::Category::OPEN_PAREN) {
                stack_size_on_open_paren.emplace_back(term_stack.size());
                term_stack.emplace_back(Term());
                ++balance_parens;
            } else if (next_token.GetCategory() ==
                       Token::Category::CLOSE_PAREN) {
                UnwindStack(term_stack, stack_size_on_open_paren,
                            bound_variables);

                --balance_parens;
            } else if (next_token.GetCategory() ==
                       Token::Category::CONSTANT_TRUE) {
                term_stack.back().Combine(Term::True());

            } else if (next_token.GetCategory() ==
                       Token::Category::CONSTANT_FALSE) {
                term_stack.back().Combine(Term::False());
            } else if (next_token.GetCategory() ==
                       Token::Category::CONSTANT_ZERO) {
                term_stack.back().Combine(Term::Zero());
            } else {
                std::ostringstream error_ss;
                error_ss << "Unexpected token: " << next_token;
                throw std::invalid_argument(error_ss.str());
            }
        }

        if (balance_parens != 0) {
            throw std::invalid_argument(
                "Invalid term: probably because a ( is not matched by a )");
        }

        while (term_stack.size() > 1) {
            CombineStackTop(term_stack);
        }

        if (term_stack.back().IsInvalid()) {
            throw std::invalid_argument("Invalid term.");
        }

        return std::move(term_stack.back());
    }

    void UnwindStack(std::vector<Term>& term_stack,
                     std::vector<int>& stack_size_on_open_paren,
                     std::vector<std::string>& bound_variables) {
        while (!term_stack.empty() && !stack_size_on_open_paren.empty() &&
               term_stack.size() > stack_size_on_open_paren.back()) {
            if (term_stack.back().IsLambda() &&
                !term_stack.back().is_complete_lambda_) {
                // Mark the λ as complete so that terms to its right
                // won't be combined to its body.
                term_stack.back().MarkLambdaAsComplete();
                // λ's variable is no longer part of the current binding
                // context, therefore pop it.
                bound_variables.pop_back();
            }

            CombineStackTop(term_stack);
        }

        if (!stack_size_on_open_paren.empty()) {
            stack_size_on_open_paren.pop_back();
        }
    }

    void CombineStackTop(std::vector<Term>& term_stack) {
        if (term_stack.size() < 2) {
            throw std::invalid_argument(
                "Invalid term: probably because a ( is not matched by a )");
        }

        Term top = std::move(term_stack.back());
        term_stack.pop_back();
        term_stack.back().Combine(std::move(top));
    }

    std::pair<std::string, Type&> ParseLambdaArg() {
        auto token = lexer_.NextToken();

        if (token.GetCategory() != Token::Category::IDENTIFIER) {
            throw std::invalid_argument("Expected to parse a variable.");
        }

        auto arg_name = token.GetText();
        token = lexer_.NextToken();

        if (token.GetCategory() != Token::Category::COLON) {
            throw std::invalid_argument("Expected to parse a ':'.");
        }

        return {arg_name, ParseType()};
    }

    Type& ParseType() {
        std::vector<Type*> parts;
        while (true) {
            auto token = lexer_.NextToken();

            if (token.GetCategory() == Token::Category::KEYWORD_BOOL) {
                parts.emplace_back(&Type::Bool());
            } else if (token.GetCategory() == Token::Category::KEYWORD_NAT) {
                parts.emplace_back(&Type::Nat());
            } else if (token.GetCategory() == Token::Category::OPEN_PAREN) {
                parts.emplace_back(&ParseType());

                if (lexer_.NextToken().GetCategory() !=
                    Token::Category::CLOSE_PAREN) {
                    std::ostringstream error_ss;
                    error_ss << __LINE__ << ": Unexpected token: " << token;
                    throw std::invalid_argument(error_ss.str());
                }
            } else if (token.GetCategory() == Token::Category::OPEN_BRACE) {
                lexer_.PutBackToken();
                parts.emplace_back(&ParseRecordType());
            } else {
                std::ostringstream error_ss;
                error_ss << __LINE__ << ": Unexpected token: " << token;
                throw std::invalid_argument(error_ss.str());
            }

            token = lexer_.NextToken();

            if (token.GetCategory() == Token::Category::DOT) {
                break;
            } else if (token.GetCategory() == Token::Category::CLOSE_PAREN ||
                       token.GetCategory() == Token::Category::CLOSE_BRACE ||
                       token.GetCategory() == Token::Category::COMMA) {
                lexer_.PutBackToken();
                break;
            } else if (token.GetCategory() != Token::Category::ARROW) {
                std::ostringstream error_ss;
                error_ss << __LINE__ << ": Unexpected token: " << token;
                throw std::invalid_argument(error_ss.str());
            }
        }

        // Types are right-associative, combine them accordingly.
        for (int i = parts.size() - 2; i >= 0; --i) {
            parts[i] = &Type::Function(*parts[i], *parts[i + 1]);
        }

        return *parts[0];
    }

    Type& ParseRecordType() {
        Token token = lexer_.NextToken();

        if (token.GetCategory() != Token::Category::OPEN_BRACE) {
            std::ostringstream error_ss;
            error_ss << __LINE__ << ": Unexpected token: " << token;
            throw std::invalid_argument(error_ss.str());
        }

        Type::RecordFields fields;
        token = lexer_.NextToken();

        while (true) {
            if (token.GetCategory() != Token::Category::IDENTIFIER) {
                std::ostringstream error_ss;
                error_ss << __LINE__ << ": Unexpected token: " << token;
                throw std::invalid_argument(error_ss.str());
            }

            std::string field_id = token.GetText();

            token = lexer_.NextToken();

            if (token.GetCategory() != Token::Category::COLON) {
                std::ostringstream error_ss;
                error_ss << __LINE__ << ": Unexpected token: " << token;
                throw std::invalid_argument(error_ss.str());
            }

            Type& type = ParseType();
            token = lexer_.NextToken();
            fields.push_back({field_id, type});

            if (token.GetCategory() == Token::Category::CLOSE_BRACE) {
                break;
            } else if (token.GetCategory() == Token::Category::COMMA) {
                token = lexer_.NextToken();
                continue;
            } else {
                std::ostringstream error_ss;
                error_ss << __LINE__ << ": Unexpected token: " << token;
                throw std::invalid_argument(error_ss.str());
            }
        }

        if (fields.empty()) {
            std::ostringstream error_ss;
            error_ss << __LINE__ << ": Invalid record.";
            throw std::invalid_argument(error_ss.str());
        }

        return Type::Record(std::move(fields));
    }

   private:
    lexer::Lexer lexer_;
};  // namespace parser
}  // namespace parser

namespace type_checker {
using parser::Term;
using parser::Type;

class TypeChecker {
    using Context = std::deque<std::pair<std::string, Type*>>;

   public:
    Type& TypeOf(const Term& term) {
        Context ctx;
        return TypeOf(ctx, term);
    }

    Type& TypeOf(const Context& ctx, const Term& term) {
        Type* res = &Type::IllTyped();

        if (term.IsTrue() || term.IsFalse()) {
            res = &Type::Bool();
        } else if (term.IsConstantZero()) {
            res = &Type::Nat();
        } else if (term.IsIf()) {
            if (TypeOf(ctx, term.IfCondition()) == Type::Bool()) {
                Type& then_type = TypeOf(ctx, term.IfThen());

                if (then_type == TypeOf(ctx, term.IfElse())) {
                    res = &then_type;
                }
            }
        } else if (term.IsSucc() || term.IsPred()) {
            auto& subterm_type = TypeOf(ctx, term.UnaryOpArg());

            if (subterm_type == Type::Nat()) {
                res = &Type::Nat();
            }
        } else if (term.IsIsZero()) {
            auto& subterm_type = TypeOf(ctx, term.UnaryOpArg());

            if (subterm_type == Type::Nat()) {
                res = &Type::Bool();
            }
        } else if (term.IsLambda()) {
            Context new_ctx =
                AddBinding(ctx, term.LambdaArgName(), term.LambdaArgType());
            Type& return_type = TypeOf(new_ctx, term.LambdaBody());
            res = &Type::Function(term.LambdaArgType(), return_type);
        } else if (term.IsApplication()) {
            Type& lhs_type = TypeOf(ctx, term.ApplicationLHS());
            Type& rhs_type = TypeOf(ctx, term.ApplicationRHS());

            if (lhs_type.IsFunction() && lhs_type.FunctionLHS() == rhs_type) {
                res = &lhs_type.FunctionRHS();
            }
        } else if (term.IsVariable()) {
            int idx = term.VariableDeBruijnIdx();

            if (idx >= 0 && idx < ctx.size() &&
                ctx[idx].first == term.VariableName()) {
                res = ctx[idx].second;
            }
        }

        return *res;
    }

   private:
    Context AddBinding(const Context& current_ctx, std::string var_name,
                       Type& type) {
        Context new_ctx = current_ctx;
        new_ctx.push_front({var_name, &type});

        return new_ctx;
    }
};
}  // namespace type_checker

namespace interpreter {
class Interpreter {
    using Term = parser::Term;

   public:
    std::pair<std::string, type_checker::Type&> Interpret(Term& program) {
        Eval(program);
        type_checker::Type& type = type_checker::TypeChecker().TypeOf(program);

        std::ostringstream ss;
        ss << program;

        auto term_str = ss.str();

        if (IsNatValue(program)) {
            std::size_t start_pos = 0;
            int num = 0;

            while ((start_pos = term_str.find("succ", start_pos)) !=
                   std::string::npos) {
                ++num;
                ++start_pos;
            }

            term_str = std::to_string(num);
        }

        return {term_str, type};
    }

   private:
    void Eval(Term& term) {
        try {
            Eval1(term);
            Eval(term);
        } catch (std::invalid_argument&) {
        }
    }

    void Eval1(Term& term) {
        auto term_subst_top = [](Term& s, Term& t) {
            // Adjust the free variables in s by increasing their static
            // distances by 1. That's because s will now be embedded one level
            // deeper in t (i.e. t's bound variable will be replaced by s).
            s.Shift(1);
            t.Substitute(0, s);
            // Because of the substitution, one level of abstraction was peeled
            // off. Account for that by decreasing the static distances of the
            // free variables in t by 1.
            t.Shift(-1);
            // NOTE: For more details see: tapl,§6.3.
        };

        if (term.IsApplication() && term.ApplicationLHS().IsLambda() &&
            IsValue(term.ApplicationRHS())) {
            term_subst_top(term.ApplicationRHS(),
                           term.ApplicationLHS().LambdaBody());
            std::swap(term, term.ApplicationLHS().LambdaBody());
        } else if (term.IsApplication() && IsValue(term.ApplicationLHS())) {
            Eval1(term.ApplicationRHS());
        } else if (term.IsApplication()) {
            Eval1(term.ApplicationLHS());
        } else if (term.IsIf()) {
            if (term.IfCondition() == Term::True()) {
                std::swap(term, term.IfThen());
            } else if (term.IfCondition() == Term::False()) {
                std::swap(term, term.IfElse());
            } else {
                Eval1(term.IfCondition());
            }
        } else if (term.IsSucc()) {
            Eval1(term.UnaryOpArg());
        } else if (term.IsPred()) {
            auto& pred_arg = term.UnaryOpArg();

            if (pred_arg.IsConstantZero()) {
                std::swap(term, term.UnaryOpArg());
            } else if (pred_arg.IsSucc()) {
                if (IsNatValue(pred_arg)) {
                    std::swap(term, pred_arg.UnaryOpArg());
                } else {
                    Eval1(pred_arg);
                }
            } else {
                Eval1(pred_arg);
            }
        } else if (term.IsIsZero()) {
            auto& iszero_arg = term.UnaryOpArg();

            if (iszero_arg.IsConstantZero()) {
                auto temp = Term::True();
                std::swap(term, temp);
            } else if (iszero_arg.IsSucc()) {
                if (IsNatValue(iszero_arg)) {
                    auto temp = Term::False();
                    std::swap(term, temp);
                } else {
                    Eval1(iszero_arg);
                }
            } else {
                Eval1(iszero_arg);
            }
        } else {
            throw std::invalid_argument("No applicable rule.");
        }
    }

    bool IsNatValue(const Term& term) {
        return term.IsConstantZero() ||
               (term.IsSucc() && IsNatValue(term.UnaryOpArg()));
    }

    bool IsValue(const Term& term) {
        return term.IsLambda() || term.IsVariable() || term.IsTrue() ||
               term.IsFalse() || IsNatValue(term);
    }
};
}  // namespace interpreter
