#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "pokemon.h"

#include <QMainWindow>

#include <QtCharts/QPolarChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QLineSeries>

#include <QComboBox>
#include <QLabel>
#include <QString>
#include <QStringList>


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
    void setComparisonData();
    void tabChanged(int index);
    void onTableClicked(int row, int column);

private:
    Ui::MainWindow *ui;
    QList<Pokemon> _pokedex;
    QStringList _types_list;
    QPolarChart* _chart;

    QList<QStringList> parsePokedexCsv();

    QList<Pokemon> getPokemonList(QList<QStringList> parsed_file);
    Pokemon createPokemon(QStringList pokemon_stats);
    QStringList getTypesList();

    void addPokemonToTable(QStringList headers);
    QString getTypeColor(QStringList types, QString type);

    void addTypeFilters(QStringList types);

    void addPokemonsToComboBox();
    void showSelectedPokemonInfo(QComboBox* pokemon_cbox, QLabel* pokemon_img, QLayout* types_layout);
    void disablePokemonFromComboBox(QComboBox* pokemon1, QComboBox* pokemon2);
    void clearLayout(QLayout* layout);

    QPolarChart* generateEmptyChart();
    void updateSeries(QPolarChart* chart, Pokemon pokemon, unsigned int brightness);
    void setSeriesColor(QLineSeries* series, Pokemon pokemon, int brightness);
    QLineSeries* generateLineSeries(Pokemon pokemon, Pokemon max);
    double getPercentageValue(unsigned int value, unsigned int max_value);
};
#endif // MAINWINDOW_H
