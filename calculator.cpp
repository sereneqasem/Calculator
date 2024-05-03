#include <iostream>
#include <stdexcept>
#include <string>
#include <map>
#include <cmath>

// Token kinds
const char number = '8';    // floating-point number
const char quit = 'q';      // exit command
const char print = ';';     // print command
const char name = 'a';      // variable name
const char let = 'L';       // variable declaration keyword
const char mod = '%';       // modulus operator

class token
{
    char kind_;
    double value_;
    std::string name_;

public:
    token(char ch)
        : kind_(ch), value_(0)
    {}

    token(double val)
        : kind_(number), value_(val)
    {}

    token(char ch, std::string n)
        : kind_(ch), name_(std::move(n))
    {}

    char kind() const { return kind_; }
    double value() const { return value_; }
    std::string name() const { return name_; }
};

class token_stream
{
    bool full;
    token buffer;

public:
    token_stream() : full(false), buffer(char(0)) {} // Changed here to explicitly cast to char

    token get();
    void putback(token t);
    void ignore(char c);
};

token_stream ts;

void token_stream::putback(token t)
{
    if (full)
        throw std::runtime_error("putback() into a full buffer");
    buffer = t;
    full = true;
}

token token_stream::get()
{
    if (full) {
        full = false;
        return buffer;
    }

    char ch;
    std::cin >> ch;

    switch (ch)
    {
    case '(': case ')': case '+': case '-': case '*': case '/': case '%':
    case ';': case '=':
        return token(ch);
    case 'q':
        return token(quit);
    case 'L':
        return token(let);
    case '.': case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    {
        std::cin.putback(ch);
        double val;
        std::cin >> val;
        return token(val);
    }
    default:
        if (std::isalpha(ch)) {
            std::string s;
            s += ch;
            while (std::cin.get(ch) && (std::isalnum(ch) || ch == '_')) s += ch;
            std::cin.putback(ch);

            if (s == "quit") return token(quit);
            return token(name, s);
        }
        throw std::runtime_error("Bad token");
    }
}

void token_stream::ignore(char c)
{
    if (full && buffer.kind() == c) {
        full = false;
        return;
    }
    full = false;

    char ch;
    while (std::cin >> ch)
        if (ch == c) return;
}

struct variable
{
    std::string name;
    double value;
};

std::map<std::string, double> symbol_table;

double get_value(std::string s)
{
    if (symbol_table.find(s) != symbol_table.end())
        return symbol_table[s];
    throw std::runtime_error("Undefined variable " + s);
}

void set_value(std::string s, double d)
{
    symbol_table[s] = d;
}

bool is_declared(std::string s)
{
    return symbol_table.find(s) != symbol_table.end();
}

double define_name(std::string var, double val)
{
    if (is_declared(var))
        throw std::runtime_error(var + " declared twice");
    symbol_table[var] = val;
    return val;
}

double expression();

double primary()
{
    token t = ts.get();
    switch (t.kind())
    {
    case '(':    // handle '(' expression ')'
    {
        double d = expression();
        t = ts.get();
        if (t.kind() != ')')
            throw std::runtime_error("')' expected");
        return d;
    }
    case '-':    // handle unary minus
        return -primary();
    case '+':    // handle unary plus
        return primary();
    case number:
        return t.value();
    case name:
        return get_value(t.name());
    default:
        throw std::runtime_error("primary expected");
    }
}


double term()
{
    double left = primary();
    while (true)
    {
        token t = ts.get();
        switch (t.kind())
        {
        case '*':
            left *= primary();
            break;
        case '/':
        {
            double d = primary();
            if (d == 0) throw std::runtime_error("divide by zero");
            left /= d;
            break;
        }
        case '%':
        {
            double d = primary();
            if (d == 0) throw std::runtime_error("divide by zero");
            left = fmod(left, d);
            break;
        }
        default:
            ts.putback(t);
            return left;
        }
    }
}

double expression()
{
    double left = term();
    while (true)
    {
        token t = ts.get();
        switch (t.kind())
        {
        case '+':
            left += term();
            break;
        case '-':
            left -= term();
            break;
        default:
            ts.putback(t);
            return left;
        }
    }
}

double declaration()
{
    token t = ts.get();
    if (t.kind() != name) throw std::runtime_error("name expected in declaration");
    std::string var_name = t.name();

    token t2 = ts.get();
    if (t2.kind() != '=') throw std::runtime_error("= missing in declaration of " + var_name);

    double d = expression();
    define_name(var_name, d);
    return d;
}

double statement()
{
    token t = ts.get();
    switch (t.kind())
    {
    case let:
        return declaration();
    default:
        ts.putback(t);
        return expression();
    }
}

void clean_up_mess()
{
    ts.ignore(print);
}

void calculate()
{
    while (true)
    {
        try
        {
            std::cout << "> ";
            token t = ts.get();
            while (t.kind() == print) t = ts.get();
            if (t.kind() == quit) return;
            ts.putback(t);
            std::cout << "= " << statement() << std::endl;
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            clean_up_mess();
        }
    }
}

int main()
{
    try
    {
        define_name("pi", 3.1415926535897932385);
        define_name("e", 2.7182818284590452354);

        calculate();
        return 0;
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "exception\n";
        return 2;
    }
}
