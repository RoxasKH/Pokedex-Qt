#ifndef POKEMON_H
#define POKEMON_H

#include <QString>
#include <QStringList>

#include <iostream>
#include <algorithm>

using namespace std;

class Pokemon
{
    QString _url_img;
    unsigned int _number;
    QString _name;
    QStringList _types;
    unsigned int _total;
    unsigned int _hp;
    unsigned int _atk;
    unsigned int _def;
    unsigned int _sp_atk;
    unsigned int _sp_def;
    unsigned int _speed;

public:

    Pokemon();

    Pokemon(
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
            int speed);

    // this aren't needed cause the compiler generated version should be enough

    /*Pokemon(const Pokemon &other);

    Pokemon &operator=(const Pokemon &other);

    ~Pokemon();

    void swap(Pokemon &other);*/

    operator QString() const;

    bool operator==(const Pokemon &pokemon) const;

    QString getUrlImg();
    unsigned int getNumber();
    QString getName();
    QStringList getTypes();
    unsigned int getTotal();
    unsigned int getHp();
    unsigned int getAtk();
    unsigned int getDef();
    unsigned int getSpAtk();
    unsigned int getSpDef();
    unsigned int getSpeed();

};

#endif // POKEMON_H
