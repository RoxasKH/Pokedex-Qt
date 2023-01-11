#include "mainwindow.h"
#include "pokemon.h"
#include "ui_mainwindow.h"

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QFile>

#include <QStringList>
#include <QDebug>
#include <QCheckBox>

#include <QStandardItem>

#include <QtCharts/QPolarChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QChartView>

#include <QString>

#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->resize(1024, 640);

    QList<QStringList> parsed_csv_file = parsePokedexCSV();

    // Extracting and separating CSV header row
    QStringList table_headers = parsed_csv_file[0];
    parsed_csv_file.removeOne(table_headers);

    foreach(QStringList row, parsed_csv_file) {
        qDebug() << row;
    }

    QList<Pokemon> pokedex = getPokemonList(parsed_csv_file);

    _pokedex = pokedex;

    addPokemonToTable(pokedex, table_headers);

    qDebug() << getTypesList(pokedex);

    connect(ui->selected_types_only, SIGNAL(clicked(bool)), this, SLOT(filter()));

    addTypeFilters(getTypesList(pokedex));

    addPokemonsToComboBox();

    // connect a SLOT to the combobox
    connect(ui->pokemon1_combobox, SIGNAL(activated(int)), this, SLOT(showPokemonInfo()));
    connect(ui->pokemon2_combobox, SIGNAL(activated(int)), this, SLOT(showPokemonInfo()));

    // hide statusbar when going to the compare tab
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged()));

    _chart = generateEmptyChart();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::filter()
{
    QStringList selected_types;
    QVBoxLayout *filters_layout = ui->filtersLayout;

    for (int i = 0; i < filters_layout->count(); ++i) {
        QCheckBox *checkbox = qobject_cast<QCheckBox *>(filters_layout->itemAt(i)->widget());
        if(checkbox->isChecked())
            selected_types.append(checkbox->text());
    }

    qDebug() << selected_types;

    //QList<Pokemon> pokedex = getPokemonList(parsed_csv_file);
    QList<Pokemon> type_filtered_pokemons;

    if(ui->selected_types_only->isChecked()) {
        foreach (Pokemon pokemon, _pokedex) {
            QStringList pokemon_types = pokemon.getTypes();
            // sort the list to be able to see if they contains the same elements
            pokemon_types.sort();
            selected_types.sort();
            if(pokemon_types==selected_types) {
                type_filtered_pokemons.append(pokemon);
                qDebug() << pokemon_types;
            }
        }
    }
    else{
        foreach (Pokemon pokemon, _pokedex) {
            foreach (QString type, selected_types) {
                if(pokemon.getTypes().contains(type))
                    type_filtered_pokemons.append(pokemon);
            }
        }
    }


    QTableWidget *table = ui->pokemonTable;

    int visible_row_count = 0;

    for(int i = 0; i < table->rowCount(); ++i) {

        table->showRow(i);

        if(!(selected_types.isEmpty())) {

            table->hideRow(i);

            foreach (Pokemon pokemon, type_filtered_pokemons) {
                if(table->item(i, 1)->text() == pokemon.getName()) {
                    table->showRow(i);
                    ++visible_row_count;
                }

            }
        }
        else{
            visible_row_count = _pokedex.size();
        }

    }

    ui->statusbar->showMessage(QStringLiteral("%1 pokemon shown.").arg(visible_row_count));

}

void MainWindow::showPokemonInfo()
{
    QComboBox *pokemon1 = ui->pokemon1_combobox;
    QComboBox *pokemon2 = ui->pokemon2_combobox;

    // disable Pokemon in a combobox if already selected in the other one

    QStandardItemModel *model1 = qobject_cast<QStandardItemModel *>(pokemon1->model());
    Q_ASSERT(model1 != nullptr);

    for(int i = 0; i < pokemon1->count(); i++) {
        QStandardItem *item = model1->item(i);
        item->setEnabled(true);
    }

    if(pokemon2->currentIndex() != -1) {
        QStandardItem *item = model1->item(pokemon2->currentIndex());
        item->setEnabled(false);
    }


    QStandardItemModel *model2 = qobject_cast<QStandardItemModel *>(pokemon2->model());
    Q_ASSERT(model2 != nullptr);

    for(int i = 0; i < pokemon2->count(); i++) {
        QStandardItem *item = model2->item(i);
        item->setEnabled(true);
    }

    if(pokemon1->currentIndex() != -1) {
        QStandardItem *item = model2->item(pokemon1->currentIndex());
        item->setEnabled(false);
    }

    // load image of the pokemon

    if(pokemon1->currentIndex() != -1) {
        foreach (Pokemon pokemon, _pokedex) {
            if(pokemon.getName() == pokemon1->currentText()) {
                ui->pokemon1_img->setStyleSheet("image: url("
                                                ":img/" + pokemon.getUrlImg() +
                                                ");"
                                                "image-position: right center;"
                                                );
                ui->pokemon1_img->setText("");

                // adding type labels

                // removing any previously loaded label by clearing the layout
                clearLayout(ui->pokemon1_types);

                foreach (QString type, pokemon.getTypes()) {
                    QLabel *type_label = new QLabel();
                    type_label->setText(type);
                    type_label->setAlignment(Qt::AlignCenter); // align the text in the label

                    type_label->setStyleSheet("QLabel { "
                                              "color: white;"
                                              "background-color : "
                                              + getTypeColor(getTypesList(_pokedex), type) +
                                              ";"
                                              "padding: 5px;"
                                              "border-radius: 3px;"
                                              "font-size: 15pt;"
                                              "}");

                    type_label->setFixedSize(100, 50);

                    ui->pokemon1_types->addWidget(type_label);
                }
            }
        }
    }

    if(pokemon2->currentIndex() != -1) {
        foreach (Pokemon pokemon, _pokedex) {
            if(pokemon.getName() == pokemon2->currentText()) {
                ui->pokemon2_img->setStyleSheet("image: url("
                                                ":img/" + pokemon.getUrlImg() +
                                                ");"
                                                "image-position: right center;"
                                                "margin: 10px;"
                                                );
                ui->pokemon2_img->setText("");

                // adding type labels

                // removing any previously loaded label by clearing the layout
                clearLayout(ui->pokemon2_types);

                foreach (QString type, pokemon.getTypes()) {
                    QLabel *type_label = new QLabel();
                    type_label->setText(type);
                    type_label->setAlignment(Qt::AlignCenter); // align the text in the label

                    type_label->setStyleSheet("QLabel { "
                                              "color: white;"
                                              "background-color : "
                                              + getTypeColor(getTypesList(_pokedex), type) +
                                              ";"
                                              "padding: 5px;"
                                              "border-radius: 3px;"
                                              "font-size: 15pt;"
                                              "}");

                    type_label->setFixedSize(100, 50);

                    ui->pokemon2_types->addWidget(type_label);
                }
            }
        }
    }

    if(pokemon1->currentIndex() != -1 && pokemon2->currentIndex() != -1) {
        updateSeries(_chart, _pokedex[pokemon1->currentIndex()], _pokedex[pokemon2->currentIndex()]);
    }

}

void MainWindow::tabChanged()
{
    QStatusBar *status_bar = ui->statusbar;

    if(ui->tabWidget->currentIndex()==0){
        status_bar->show();
    }
    if(ui->tabWidget->currentIndex()==1){
        status_bar->hide();
    }
}

void MainWindow::addPokemonToTable(QList<Pokemon> pokedex, QStringList headers)
{
    QTableWidget *table = ui->pokemonTable;

    QFont font;
    font.setBold(true);

    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSortingEnabled(true);

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    table->setRowCount(pokedex.size());
    table->setColumnCount(headers.size()-1);

    headers.removeAt(0);
    table->setHorizontalHeaderLabels(headers);

    table->verticalHeader()->hide();
    table->horizontalHeader()->setFont(font);

    unsigned int row_count = 0;
    foreach(Pokemon pokemon, pokedex) {

        QTableWidgetItem *number = new QTableWidgetItem;
        number->setData(Qt::EditRole, pokemon.getNumber()); // this puts an int in the table so it can sort by number
        table->setItem(row_count, 0 ,number);
        QTableWidgetItem *name = new QTableWidgetItem;
        name->setText(pokemon.getName()); // this instead puts a string, so it'll sort by character
        table->setItem(row_count, 1 ,name);

        QTableWidgetItem* types = new QTableWidgetItem();
        table->setItem(row_count, 2, types);// set an empty item to the table cell

        QWidget *colorTableWidget = new QWidget();

        QVBoxLayout *types_layout = new QVBoxLayout();

        foreach (QString type, pokemon.getTypes()) {
            QLabel *type_label = new QLabel();
            type_label->setText(type);
            type_label->setAlignment(Qt::AlignCenter); // align the text in the label

            type_label->setStyleSheet("QLabel { "
                                      "color: white;"
                                      "background-color : "
                                      + getTypeColor(getTypesList(pokedex), type) +
                                      ";"
                                      "padding: 5px;"
                                      "border-radius: 3px;"
                                      "}");

            types_layout->addWidget(type_label);
        }

        colorTableWidget->setLayout(types_layout);
        table->setCellWidget(row_count, 2 ,colorTableWidget);

        QTableWidgetItem *total = new QTableWidgetItem;
        total->setData(Qt::EditRole, pokemon.getTotal());
        table->setItem(row_count, 3 ,total);
        QTableWidgetItem *hp = new QTableWidgetItem;
        hp->setData(Qt::EditRole, pokemon.getHp());
        table->setItem(row_count, 4 ,hp);
        QTableWidgetItem *atk = new QTableWidgetItem;
        atk->setData(Qt::EditRole, pokemon.getAtk());
        table->setItem(row_count, 5 ,atk);
        QTableWidgetItem *def = new QTableWidgetItem;
        def->setData(Qt::EditRole, pokemon.getDef());
        table->setItem(row_count, 6 ,def);
        QTableWidgetItem *spatk = new QTableWidgetItem;
        spatk->setData(Qt::EditRole, pokemon.getSpAtk());
        table->setItem(row_count, 7 ,spatk);
        QTableWidgetItem *spdef = new QTableWidgetItem;
        spdef->setData(Qt::EditRole, pokemon.getSpDef());
        table->setItem(row_count, 8 ,spdef);
        QTableWidgetItem *speed = new QTableWidgetItem;
        speed->setData(Qt::EditRole, pokemon.getSpeed());
        table->setItem(row_count, 9 ,speed);

        ++row_count;
    }

    for(int i = 0; i < table->rowCount(); ++i) {
        for(int j = 3; j < table->columnCount(); ++j) {
            table->item(i,j)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }
    }

    for(int i = 0; i < table->rowCount(); ++i) {
        table->item(i,0)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        table->item(i, 2)->setData(Qt::TextAlignmentRole, Qt::AlignCenter);
        table->item(i, 3)->setFont(font);
    }

    table->resizeRowsToContents();
    //table->resizeColumnToContents(2);

    ui->statusbar->showMessage(QStringLiteral("%1 pokemon shown.").arg(pokedex.size()));

}

QList<QStringList> MainWindow::parsePokedexCSV()
{

    QString PokedexCSVPath = ":/csv/pokedex.csv"; // path of the pokedex csv resource

    QFile file(PokedexCSVPath); // create a QFile from the csv file at the PokedexCSVPath
    QStringList wordList; // the list of string where i'm gonna save the csv lines

    QTextStream stream(&file); // open a new stream from the QFile

    //qDebug() << file.exists(); // check if the file really exists at that path

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) { // if the file opens correctly
        while(!stream.atEnd()) { // until the file textstream doesn't reach its end
            QString line = stream.readLine(); // take a line as a string and go to the next
            wordList.append(line); // append the line to the wordList created before
        }
    }
    else { // do something if it can't open the file
        qDebug() << "File exists: " << file.exists();
        qDebug() << "Error string: " << file.errorString();
        qDebug() << "QFileDevice::FileError error value: " << file.error() << " - see: https://doc.qt.io/qt-5/qfiledevice.html#FileError-enum";
    }

    QStringList CSVrow;

    QList<QStringList> pokemon_infos;

    // Fixing type separator from "," to "|" if the pokemon have multiple types
    for(QString& row: wordList) {
        CSVrow = row.split('"');
        if(CSVrow.size() > 1) { // this occurs only if the pokemon have multiple types or the line isn't the headers one
            CSVrow[1].replace(",", "|");
            CSVrow[1] = CSVrow[1].simplified().remove(" "); // trim the types string from any whitespaces
        }
        row = CSVrow.join("");
    }

    foreach(QString row, wordList) {

        CSVrow = row.split(","); // split it into a list of string using "," as a separator
        pokemon_infos.append(CSVrow); // append the list to a list of QStringList
    }

    return pokemon_infos;
}

QList<Pokemon> MainWindow::getPokemonList(QList<QStringList> parsed_file)
{
    QList<Pokemon> pokemon_list;

    foreach(QStringList pokemon_stats, parsed_file) {
        pokemon_list.append(createPokemon(pokemon_stats));
    }

    foreach(Pokemon pokemon, pokemon_list) {
        qDebug()
                << pokemon.getUrlImg()
                << ","
                << pokemon.getNumber()
                << ","
                << pokemon.getName()
                << ","
                << pokemon.getTypes()
                << ","
                << pokemon.getTotal()
                << ","
                << pokemon.getHp()
                << ","
                << pokemon.getAtk()
                << ","
                << pokemon.getDef()
                << ","
                << pokemon.getSpAtk()
                << ","
                << pokemon.getSpDef()
                << ","
                << pokemon.getSpeed();
    }

    return pokemon_list;
}

Pokemon MainWindow::createPokemon(QStringList pokemon_stats)
{
    QString url_img = pokemon_stats[0];
    unsigned int number = pokemon_stats[1].toInt();
    QString name = pokemon_stats[2];
    QStringList types = pokemon_stats[3].split("|");
    unsigned int total = pokemon_stats[4].toInt();
    unsigned int hp = pokemon_stats[5].toInt();
    unsigned int atk = pokemon_stats[6].toInt();
    unsigned int def = pokemon_stats[7].toInt();
    unsigned int sp_atk = pokemon_stats[8].toInt();
    unsigned int sp_def = pokemon_stats[9].toInt();
    unsigned int speed = pokemon_stats[10].toInt();

    Pokemon tmp(url_img, number, name, types, total, hp, atk, def, sp_atk, sp_def, speed);

    return tmp;
}

QStringList MainWindow::getTypesList(QList<Pokemon> pokedex) {
    QStringList types_list;

    foreach (Pokemon pokemon, pokedex) {
        foreach(QString type, pokemon.getTypes())
            if(!types_list.contains(type)) // save a type only if not already present in the list
                types_list.append(type);
    }

    return types_list;
}

void MainWindow::addTypeFilters(QStringList types)
{
    QVBoxLayout *filters_layout = ui->filtersLayout;
    foreach (QString type, types) {
        QCheckBox *type_filter = new QCheckBox;
        type_filter->setText(type);
        filters_layout->addWidget(type_filter);
        connect(type_filter, SIGNAL(clicked(bool)), this, SLOT(filter()));
    }

}

QString MainWindow::getTypeColor(QStringList types, QString type)
{
    switch(types.indexOf(type)) {
        case 0: return "#79BF53"; // 0 corresponds to the first element of the QStringList, so on
            break;
        case 1: return "#984E94";
            break;
        case 2: return "#E97F34";
            break;
        case 3: return "#A890F0";
            break;
        case 4: return "#754EC7";
            break;
        case 5: return "#6890F0";
            break;
        case 6: return "#A8B820";
            break;
        case 7: return "#A8A878";
            break;
        case 8: return "#705848";
            break;
        case 9: return "#F8D030";
            break;
        case 10: return "#F85888";
            break;
        case 11: return "#E0C068";
            break;
        case 12: return "#98D8D8";
            break;
        case 13: return "#B8B8D0";
            break;
        case 14: return "#F0B6BC";
            break;
        case 15: return "#C03028";
            break;
        case 16: return "#B8A038";
            break;
        case 17: return "#705898";
            break;
        default: return "white; border: 1px solid grey; color: black;";
    }
}

void MainWindow::addPokemonsToComboBox()
{
    QComboBox *pokemon1 = ui->pokemon1_combobox;
    QComboBox *pokemon2 = ui->pokemon2_combobox;

    foreach (Pokemon pokemon, _pokedex) {
        pokemon1->addItem(pokemon.getName());
        pokemon2->addItem(pokemon.getName());
    }

    pokemon1->setPlaceholderText(QStringLiteral("--Select a Pokemon--"));
    pokemon1->setCurrentIndex(-1);

    pokemon2->setPlaceholderText(QStringLiteral("--Select a Pokemon--"));
    pokemon2->setCurrentIndex(-1);
}

void MainWindow::clearLayout(QLayout* layout)
{
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (QWidget* widget = item->widget()) {
            delete widget;
        }
    }
}

QPolarChart* MainWindow::generateEmptyChart()
{

    QPolarChart *chart = new QPolarChart();

    QChartView *chartView = new QChartView();
    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    ui->chart_layout->addWidget(chartView);

    return chart;

}

void MainWindow::clearChart(QPolarChart *chart)
{
    chart->removeAllSeries();
}

double MainWindow::getPercentageValue(unsigned int value, unsigned int max_value)
{
    return static_cast<double>(value)/max_value*100;
}

void MainWindow::updateSeries(QPolarChart* chart, Pokemon pokemon1, Pokemon pokemon2)
{
    clearChart(chart);

    unsigned int max_total = 0;
    unsigned int max_hp = 0;
    unsigned int max_atk = 0;
    unsigned int max_def = 0;
    unsigned int max_sp_atk = 0;
    unsigned int max_sp_def = 0;
    unsigned int max_speed = 0;

    foreach (Pokemon pokemon, _pokedex) {
        if(pokemon.getTotal()>max_total) {max_total = pokemon.getTotal();}
        if(pokemon.getHp()>max_hp) {max_hp = pokemon.getHp();}
        if(pokemon.getAtk()>max_atk) {max_atk = pokemon.getAtk();}
        if(pokemon.getDef()>max_def) {max_def = pokemon.getDef();}
        if(pokemon.getSpAtk()>max_sp_atk) {max_sp_atk = pokemon.getSpAtk();}
        if(pokemon.getSpDef()>max_sp_def) {max_sp_def = pokemon.getSpDef();}
        if(pokemon.getSpeed()>max_speed) {max_speed = pokemon.getSpeed();}
    }

    QCategoryAxis *angularAxis = new QCategoryAxis; // the line axis
    angularAxis->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    angularAxis->setRange(0, 360);
    angularAxis->append("Total", 0);
    angularAxis->append("Speed", 1*(360/7));
    angularAxis->append("Sp. Def", 2*(360/7));
    angularAxis->append("Sp. Atk", 3*(360/7));
    angularAxis->append("Defense", 4*(360/7));
    angularAxis->append("Attack", 5*(360/7));
    angularAxis->append("HP", 6*(360/7));

    chart->addAxis(angularAxis, QPolarChart::PolarOrientationAngular);

    QValueAxis *radialAxis = new QValueAxis(); // the circular axis
    radialAxis->setTickCount(9); // tick on the axis
    radialAxis->setLabelFormat("%d");
    radialAxis->setRange(0, 100);
    chart->addAxis(radialAxis, QPolarChart::PolarOrientationRadial);


    QLineSeries *series1 = new QLineSeries();
    series1->append(0, getPercentageValue(pokemon1.getTotal(), max_total));
    series1->append(1*(360/7), getPercentageValue(pokemon1.getSpeed(), max_speed));
    series1->append(2*(360/7), getPercentageValue(pokemon1.getSpDef(), max_sp_def));
    series1->append(3*(360/7), getPercentageValue(pokemon1.getSpAtk(), max_sp_atk));
    series1->append(4*(360/7), getPercentageValue(pokemon1.getDef(), max_def));
    series1->append(5*(360/7), getPercentageValue(pokemon1.getAtk(), max_atk));
    series1->append(6*(360/7), getPercentageValue(pokemon1.getHp(), max_hp));
    series1->append(360, getPercentageValue(pokemon1.getTotal(), max_total));

    QLineSeries *series2 = new QLineSeries();
    series2->append(0, getPercentageValue(pokemon2.getTotal(), max_total));
    series2->append(1*(360/7), getPercentageValue(pokemon2.getSpeed(), max_speed));
    series2->append(2*(360/7), getPercentageValue(pokemon2.getSpDef(), max_sp_def));
    series2->append(3*(360/7), getPercentageValue(pokemon2.getSpAtk(), max_sp_atk));
    series2->append(4*(360/7), getPercentageValue(pokemon2.getDef(), max_def));
    series2->append(5*(360/7), getPercentageValue(pokemon2.getAtk(), max_atk));
    series2->append(6*(360/7), getPercentageValue(pokemon2.getHp(), max_hp));
    series2->append(360, getPercentageValue(pokemon2.getTotal(), max_total));

    chart->addSeries(series1);
    chart->addSeries(series2);

    chart->addAxis(angularAxis, QPolarChart::PolarOrientationAngular);
    chart->addAxis(radialAxis, QPolarChart::PolarOrientationRadial);

    series1->attachAxis(radialAxis);
    series1->attachAxis(angularAxis);
    series2->attachAxis(radialAxis);
    series2->attachAxis(angularAxis);

}


