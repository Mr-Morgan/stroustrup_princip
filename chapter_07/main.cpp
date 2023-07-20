#include "std_lib_facilities.h"
// Exercise (1+,2+,3+,4+,5+,6+,7)
//------------------------------------------------------------------------------
/* TEST DATA
1+2 q
()
~~
!1~~
-1;
-7+8;
-99+55*(-2);
3+8%5-22*(-9/3);
1@z; 1+3;
~~
let x = 3.4;
let y = 2;
let z = (x+y*2)/pi;
let z1 = -y*2+z*pi;
sqrt(z1);
let x = 3.4;
let y = 2;
let z1 = x + y * 2;
let z2 = x+y*2;
3+5;
q
*/
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

constexpr char number       = '8';  // t.kind==number обозначает число в классе Token
constexpr char quit         = 'q';  // t.kind==quit обозначает выход в классе Token
constexpr char print        = ';';  // t.kind==print обозначает вывод на печать в классе Token
constexpr char name         = 'a';  // t.kind==name обозначает имя переменной в классе Token
constexpr char let          = 'L';  // t.kind==let обозначает объявление переменной
constexpr char square_root  = 'S';  // t.kind==square_root обозначает квадратный корень
const string sqrtkey    = "sqrt";   // sqrtkey ключевое слово используемое для вычисления квадратного корня
const string declkey    = "let";    // declkey ключевое слово используемое для объявления переменной
const string prompt     = "> ";     // приглашение к в вводу
const string result     = "= ";     // используется при выводе результата

//------------------------------------------------------------------------------

class Token {
public:
    char kind;        // тип токена
    double value;     // значение, для чисел
    string name;      // для имен переменных
    Token(char ch)              : kind(ch), value(0)    {}
    Token(char ch, double val)  : kind(ch), value(val)  {}
    Token(char ch, string n)    : kind(ch), name(n)     {}
};

//------------------------------------------------------------------------------

class Token_stream {
public:
    Token_stream();             // конструктор
    Token get();                // объявление метода позволяющего возвращать из потока Token
    void putback(Token t);      // объявление метода позволяющего возвращать Token в поток
    void ignore(char c);
private:
    bool full;                  // буфер заполнен?
    Token buffer;               // используется для хранения возвращенного в поток объекта Token
};

//------------------------------------------------------------------------------
// конструктор класса Token_stream
Token_stream::Token_stream()
    :full(false), buffer(0) {}  // буфер пуст

//------------------------------------------------------------------------------

// метод putback() позволяющий возвращать Token в буфер потока Token_stream:
void Token_stream::putback(Token t)
{   // заполнят только когда буфер пуст
    if (full) error("putback() into a full buffer");
    buffer = t;
    full = true;                // буфер полон
}//void Token_stream::putback(Token t)

//------------------------------------------------------------------------------
// метод считывающий символы из cin в поток Token_stream с объектами Token
Token Token_stream::get()
{
    if (full) {        // проверка наличия значения возвращенного в поток (буфер)
        full=false;
        return buffer;
    }// if (full)

    char ch;
    cin >> ch;    // оператор >> пропускает пробелы табуляции и т.п.

    switch (ch) {
    case print:                     // для "print"
    case quit:                      // для "quit"
    case '!':                       // для вычисления факториала
    case '%':                       // остаток от деления
    case '=':                       // для объявления значения переменной
    case '(': case ')':
    case '{': case '}':
    case '+': case '-':
    case '*': case '/':
        return Token(ch);           // возвращает Token содержащий лишь символ
    case '.':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    {
        cin.putback(ch);            // первая цифра числа возвращается в поток
        double val;
        cin >> val;                 // и считывается как число с плавающей точкой
        return Token(number,val);   // возвращает Token содержащий тип и значение числа
    }
    default:
        if (isalpha(ch)) { // имя переменной или ключевое слово ("let") начинается только с буквы
            string s;
            s += ch;
            // имя переменной может содержать буквы и цифры в произвольном порядке со второго символа
            while (cin.get(ch) && (isalpha(ch) || isdigit(ch) || (ch == '_'))) s+=ch;
            // последний считанный символ не являющийся буквой или цифрой возвращается в поток
            cin.putback(ch);
            if (s == declkey) return Token(let); // ключевое слово "let"
            else if (s == sqrtkey)
                return Token(square_root);
            return Token(name,s);
        }//if (isalpha(ch))
        error("Bad token");
    }// switch (ch)
}// Token Token_stream::get()

//------------------------------------------------------------------------------
// метод ignore класса Token_stream
void Token_stream::ignore(char c)
{ // игнорирует символ с в потоке
    if (full && c == buffer.kind) {// проверка буфера:
        full = false;
        return;
    }//if (full && c == buffer.kind)
    // чтобы игнорировать символ из буфера != с при последующей нормальной работе
    full = false;
    // поиск в потоке:
    char ch = '?';
    while (cin >> ch)
        if (ch == c) return;
}// void Token_stream::ignore(char c)

//------------------------------------------------------------------------------
// класс для хранения переменных
class Variable {
public:
    string name;
    double value;
    Variable(string n, double v) : name(n), value(v) {}
};

//------------------------------------------------------------------------------
// static - область видимости ограничена файлом, помогает избежать конфликта имен
static Token_stream ts;                    // обеспечивает get() и putback()
static vector<Variable> var_table;

//------------------------------------------------------------------------------
// метод возвращающий значение переменной по его имени
double get_value(string s)
{
    for (const Variable& v : var_table)
        if (v.name == s) return v.value;
    error("get: unkhown variable ", s);
}// double get_value(string s)

//------------------------------------------------------------------------------
// метод устанавливающий значение переменной по его имени
void set_value(string s, double d)
{
    for (Variable& v : var_table)
        if (v.name == s) {
            v.value = d;
            return;
        }// if (v.name == s)
    error("set: unkhown variable ", s);
}// void set_value(string s, double d)

//------------------------------------------------------------------------------
// метод определяющий была ли объявлена переменная с таким именем
bool is_declared(string var)
{
    for (int i = 0; i < var_table.size(); ++i)
        if (var_table[i].name == var) return true;
    return false;
}// bool is_declared(string var)

//------------------------------------------------------------------------------
// метод объявляющий переменную var со значением val
double define_name(string var, double val)
{
    if (is_declared(var)) error(var," declared twice");
    var_table.push_back(Variable(var,val));
    return val;
}// double define_name(string var, double val)

//------------------------------------------------------------------------------
// объявление метода expression для его вызова в методе primary
double expression();

//------------------------------------------------------------------------------
// метод возвращающий факториал числа а
int factorial(int a)
{
    if (a == 0) return 1;
    return (a--)*factorial(a);
}// int factorial(int a)

//------------------------------------------------------------------------------
// первичное выражение. содержит число, (выражение), переменную
double primary()
{
    Token t = ts.get();
    switch (t.kind) {
    case '(':                       // handle '(' expression ')'
    {
        double d = expression();
        t = ts.get();
        if (t.kind != ')') error("')' expected");
        return d;
    }
    case number:
        return t.value;             // return the number's value
    case name:
        return get_value(t.name);   // return the variable's value
    case '-':
        return - primary();
    case '+':
        return primary();
    default:
        error("primary expected");
    }// switch (t.kind)
}// double primary()

//------------------------------------------------------------------------------
// вторичное выражение. содержит {выражение}
double secondary()
{//-----верна ли логика? не стоит ли выполнять проверку на одном уровне с let???
    // Test data // {(4+5)*6}/(3+4); //

    Token t = ts.get();
    switch (t.kind) {
    case '{':    // handle '(' expression ')'
        {
            double d = expression();
            t = ts.get();
            if (t.kind != '}') error("'}' expected");
            return d;
        }
    case square_root:
    {
        t = ts.get();
        if (t.kind != '(') error("'(' expected");
        double d = expression();
        t = ts.get();
        if (t.kind != ')') error("')' expected");
        return sqrt(d);
        break;
    }
    default:
        ts.putback(t);
        return primary();
    }// switch (t.kind)
}// double secondary()

//------------------------------------------------------------------------------
// третичное выражение. содержит факториал числа !
double tertiary()
{
    double left = secondary();
    Token t = ts.get();
    switch (t.kind) {
    case '!':
    {   // вычисляет факториал, если left целое
        int i1 = narrow_cast<int>(left);
        if (left-i1 == 0.0) return factorial(i1);
        else error("Факториал определен только для натуральных чисел!");
    }
    default:
        ts.putback(t);
        return left;
    }// switch (t.kind)
}// double tertiary()

//------------------------------------------------------------------------------
// терм. содержит операции умножения, деления и остатка от деления для целых чисел
double term()
{
    double left = tertiary();
    Token t = ts.get();

    while(true) {
        switch (t.kind) {
        case '*':
        {
            left *= tertiary();
            t = ts.get();
            break;
        }
        case '/':
        {
            double d = tertiary();
            if (d == 0) error("divide by zero");
            left /= d;
            t = ts.get();
            break;
        }
        case '%':
        {
            int i1 = narrow_cast<int>(left);
            int i2 = narrow_cast<int>(term());
            if (i2 == 0) error("%: divide by zero");
            left = i1%i2;
            t = ts.get();
            break;
        }
        default:
            ts.putback(t);     // put t back into the token stream
            return left;
        }// switch (t.kind)
    }// while(true)
}// double term()

//------------------------------------------------------------------------------
// выражение. содержит операции сложения и вычитания
double expression()
{
    double left = term();
    Token t = ts.get();

    while(true) {
        switch(t.kind) {
        case '+':
            left += term();
            t = ts.get();
            break;
        case '-':
            left -= term();
            t = ts.get();
            break;
        default:
            ts.putback(t);
            return left;
        }// switch(t.kind)
    }// while(true)
}// double expression()

//------------------------------------------------------------------------------
// объявление переменной var_name с записью ее значения expression()
double declaration()
{
    Token t = ts.get();
    if (t.kind != name) error ("name expected in declaration");
    string var_name = t.name;

    Token t2 = ts.get();
    if (t2.kind != '=') error("= missing in declaration of ", var_name);

    double d = expression();
    define_name(var_name,d);
    return d;
}// double declaration()

//------------------------------------------------------------------------------
// утверждение. если переменная, то объявление, в противном случае вычисление выражения
double statement()
{
    Token t = ts.get();
    switch (t.kind) {
    case let:
        return declaration();
    default:
        ts.putback(t);
        return expression();
    }// switch (t.kind)
}// double statement()

//------------------------------------------------------------------------------
// в случае ошибки обработке не подвергаются все символы до ближайшей ;
void clean_up_mess()
{
    ts.ignore(print);
}// void clean_up_mess()

//------------------------------------------------------------------------------
// вычисление введенных значений
void calculate()
{
    while (cin)
        try {
            cout << prompt;
            Token t = ts.get();
            while (t.kind == print) t = ts.get();
            if (t.kind == quit) return;
            ts.putback(t);
            cout << endl << result << statement() << '\n';
        } catch (exception& e) {
        cerr << e.what() << '\n';
        clean_up_mess();
        }
}// void calculate()
//------------------------------------------------------------------------------

int main()
try
{
    // объявление констант
    define_name("pi", 3.1415926535);
    define_name("e",  2.7182818284);
    define_name("k",  1e3);

    cout << "\t   Добро пожаловать в программу-калькулятор!\n"
            "\tВводите выражения с числами с плавающей точкой\n"
            "\tразделяя их ;. Для вычислений используйте +, -,\n"
            "\t *, /, (, ). Для выхода используйте команду q\n"
            "";
    calculate();
    keep_window_open("~~");
    return 0;
}// try
catch (exception& e) {
    cerr << "error: " << e.what() << '\n';
    keep_window_open("~~");
    return 1;
}// catch (exception& e)
catch (...) {
    cerr << "Oops: unknown exception!\n";
    keep_window_open("~~");
    return 2;
}// catch (...)

//------------------------------------------------------------------------------
