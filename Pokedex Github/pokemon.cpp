#include "pokemon.h"

Pokemon::Pokemon() :
    _url_img(""),
    _number(0),
    _name(""),
    _types({""}),
    _total(0),
    _hp(0),
    _atk(0),
    _def(0),
    _sp_atk(0),
    _sp_def(0),
    _speed(0)
{}

Pokemon::Pokemon(
        QString url_img,
        int number,
        QString name,
        QStringList types,
        int total,
        int hp,
        int atk,
        int def,
        int sp_atk,
        int sp_def,
        int speed)
    :
    _url_img(url_img),
    _number(number),
    _name(name),
    _types(types),
    _total(total),
    _hp(hp),
    _atk(atk),
    _def(def),
    _sp_atk(sp_atk),
    _sp_def(sp_def),
    _speed(speed)
{}

/*Pokemon::Pokemon(const Pokemon &other)
    :
      _url_img(other._url_img),
      _number(other._number),
      _name(other._name),
      _types(other._types),
      _total(other._total),
      _hp(other._hp),
      _atk(other._atk),
      _def(other._def),
      _sp_atk(other._sp_atk),
      _sp_def(other._sp_def),
      _speed(other._speed)
{}

Pokemon& Pokemon::operator=(const Pokemon &other) {
    if(this != &other) {
        Pokemon tmp(other);
        this->swap(tmp);
    }

    return *this;
}

Pokemon::~Pokemon(){}

void Pokemon::swap(Pokemon &other) {
    ::swap(_url_img, other._url_img);
    ::swap(_number, other._number);
    ::swap(_name, other._name);
    ::swap(_types, other._types);
    ::swap(_total, other._total);
    ::swap(_hp, other._hp);
    ::swap(_atk, other._atk);
    ::swap(_def, other._def);
    ::swap(_sp_atk, other._sp_atk);
    ::swap(_sp_def, other._sp_def);
    ::swap(_speed, other._speed);
}*/

QString Pokemon::getUrlImg() {return _url_img;}
unsigned int Pokemon::getNumber() {return _number;}
QString Pokemon::getName() {return _name;}
QStringList Pokemon::getTypes() {return _types;}
unsigned int Pokemon::getTotal() {return _total;}
unsigned int Pokemon::getHp() {return _hp;}
unsigned int Pokemon::getAtk() {return _atk;}
unsigned int Pokemon::getDef() {return _def;}
unsigned int Pokemon::getSpAtk(){return _sp_atk;}
unsigned int Pokemon::getSpDef() {return _sp_def;}
unsigned int Pokemon::getSpeed() {return _speed;}
