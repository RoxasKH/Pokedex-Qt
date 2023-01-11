#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "pokemon.h"

#include <QMainWindow>

#include <QtCharts/QPolarChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void filter();
    void showPokemonInfo();
    void tabChanged();

private:
    QList<Pokemon> _pokedex;
    QPolarChart *_chart;

    Ui::MainWindow *ui;
    void addPokemonToTable(QList<Pokemon> pokemon_list, QStringList headers);
    QList<QStringList> parsePokedexCSV();
    QList<Pokemon> getPokemonList(QList<QStringList> parsed_file);
    Pokemon createPokemon(QStringList pokemon_stats);
    QStringList getTypesList(QList<Pokemon> pokedex);
    void addTypeFilters(QStringList types);
    QString getTypeColor(QStringList types, QString type);
    void addPokemonsToComboBox();
    void clearLayout(QLayout* layout);
    QPolarChart* generateEmptyChart();
    void clearChart(QPolarChart *chart);
    double getPercentageValue(unsigned int value, unsigned int max_value);
    void updateSeries(QPolarChart* chart, Pokemon pokemon1, Pokemon pokemon2);
};
#endif // MAINWINDOW_H
