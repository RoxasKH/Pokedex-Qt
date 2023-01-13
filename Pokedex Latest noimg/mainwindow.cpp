#include "mainwindow.h"
#include "pokemon.h"
#include "ui_mainwindow.h"

#include <QTableWidget>
#include <QFile>
#include <QCheckBox>
#include <QStandardItem>

#include <QLineEdit>

#include <QtCharts/QChartView>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("Pokedex");

    this->resize(1024, 640);

    QList<QStringList> parsed_csv_file = parsePokedexCsv();

    // extracting and separating CSV header row
    QStringList table_headers = parsed_csv_file[0];
    parsed_csv_file.removeOne(table_headers);

    // print the pokemon list to console
    foreach(QStringList row, parsed_csv_file) {
        qDebug() << row;
    }

    // convert the QList<QStringList> of pokemons to a QList<Pokemon> and assign it to _pokedex
    _pokedex = getPokemonList(parsed_csv_file);

    // getting the types list from the pokedex and assign it to _types_list
    _types_list = getTypesList();

    // add the whole pokemon list to the table
    addPokemonToTable(table_headers);

    // print the types_list to console
    qDebug() << _types_list;

    // make the searchbar send a signal to the filter slot everytime the text inside is edited
    connect(ui->search_lineedit, SIGNAL(textChanged(QString)), this, SLOT(filter()));

    // connect the filter slot to the hardcoded selected_types_only checkbox as well
    connect(ui->selected_types_only, SIGNAL(clicked(bool)), this, SLOT(filter()));

    // generate filter checkboxes for each existing type and load them in the filters layout
    addTypeFilters(_types_list);

    // add all the pokemons to both the comboboxes in the compare tab
    addPokemonsToComboBox();

    // connect the comboboxes to the showPokemonInfo slot which will load images, types and update the graph
    connect(ui->pokemon1_combobox, SIGNAL(activated(int)), this, SLOT(setComparisonData()));
    connect(ui->pokemon2_combobox, SIGNAL(activated(int)), this, SLOT(setComparisonData()));

    // hide the program statusbar when switching to the compare tab
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    // generate the empty polarGraph
    _chart = generateEmptyChart();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::filter()
{
    // retrieving the type checkboxes that have been selected in the type filter layout as a QStringList
    QStringList selected_types;
    QVBoxLayout *filters_layout = ui->filtersLayout;

    for (int i = 0; i < filters_layout->count(); ++i) {
        QCheckBox *checkbox = qobject_cast<QCheckBox *>(filters_layout->itemAt(i)->widget());
        if(checkbox->isChecked())
            selected_types.append(checkbox->text());
    }

    // printing the selected types list
    qDebug() << selected_types;

    // getting the filtered pokemon list
    QList<Pokemon> type_filtered_pokemons;

    // if there is at least one type selected
    if(!(selected_types.isEmpty())) {
        // if the selected_types_only flag is checked, get only the ones who's types list is equal to selected_types
        if(ui->selected_types_only->isChecked()) {
            foreach (Pokemon pokemon, _pokedex) {
                QStringList pokemon_types = pokemon.getTypes();
                // sort the list to be able to see if they contains the same elements
                pokemon_types.sort();
                selected_types.sort();
                if(pokemon_types == selected_types) {
                    type_filtered_pokemons.append(pokemon);
                }
            }
        }
        else{ // otherwise get the ones which types list contains at least one of the types in selected_types
            foreach (Pokemon pokemon, _pokedex) {
                foreach (QString type, selected_types) {
                    if(pokemon.getTypes().contains(type))
                        type_filtered_pokemons.append(pokemon);
                }
            }
        }
    }
    else {
        // if no types are selected, set the type filtered list (which still needs text filtering) to the whole pokedex
        type_filtered_pokemons = _pokedex;
    }

    // get the filtered pokemon list (by name/number + type)
    QList<Pokemon> filtered_pokemons;
    // append to the list any pokemon that have the text written in the searchbar as substring
    // or that has the text written as its number in the pokedex
    foreach (Pokemon pokemon, type_filtered_pokemons) {
        QString search_text = ui->search_lineedit->text();
        if(pokemon.getName().contains(search_text, Qt::CaseInsensitive) ||
                QString::number(pokemon.getNumber()) == search_text)
            filtered_pokemons.append(pokemon);
    }

    // getting the table from the ui
    QTableWidget *table = ui->pokemonTable;

    for(int i = 0; i < table->rowCount(); ++i) {

        // show all rows
        table->showRow(i);

        // if at least one type is selected int the filter layout and the serach lineedit isn't empty
        if(!(selected_types.isEmpty() && ui->search_lineedit->text() == "" )) {
            // hide all rows
            table->hideRow(i);
            // show every row that contains the name of a type-filtered pokemon
            foreach (Pokemon pokemon, /*type_*/filtered_pokemons) {
                if(table->item(i, 1)->text() == pokemon.getName()) {
                    table->showRow(i);
                }
            }
        }

    }

    // update statusbar message with the number of visible rows (equal to the size of the filtered list)
    ui->statusbar->showMessage(QStringLiteral("%1 pokemon shown.").arg(filtered_pokemons.size()));

}

void MainWindow::setComparisonData()
{
    QComboBox *combobox1 = ui->pokemon1_combobox;
    QComboBox *combobox2 = ui->pokemon2_combobox;

    QLabel *pokemon1_img = ui->pokemon1_img;
    QLabel *pokemon2_img = ui->pokemon2_img;

    QHBoxLayout *types_layout1 = ui->pokemon1_types;
    QHBoxLayout *types_layout2 = ui->pokemon2_types;

    // disable pokemon selected in combobox2 in combobox1
    disablePokemonFromComboBox(combobox1, combobox2);
    // disable pokemon selected in combobox1 in combobox2
    disablePokemonFromComboBox(combobox2, combobox1);

    // showing images and type labels for selected pokemons
    showSelectedPokemonInfo(combobox1, pokemon1_img, types_layout1);
    showSelectedPokemonInfo(combobox2, pokemon2_img, types_layout2);

    // remove all old series attached to the chart
    _chart->removeAllSeries();

    // if both pokemons are selected update the lineseries on the polarchart with the correspondent pokemon stats
    if(combobox1->currentIndex() != -1 && combobox2->currentIndex() != -1) {
        Pokemon pokemon1 = _pokedex[combobox1->currentIndex()];
        Pokemon pokemon2 = _pokedex[combobox2->currentIndex()];
        updateSeries(_chart, pokemon1, pokemon2);
    }

}

void MainWindow::showSelectedPokemonInfo(QComboBox* pokemon_cbox, QLabel* pokemon_img, QLayout* types_layout)
{
    // removing any previously possible loaded label by clearing the type layout
    clearLayout(types_layout);
    pokemon_img->setStyleSheet("");
    pokemon_img->setText("No Pokemon Selected");

    // if a pokemon is selected in the combobox
    if(pokemon_cbox->currentIndex() != -1) {
        // retrieve the selected pokemon from the _pokedex list
        Pokemon pokemon = _pokedex[pokemon_cbox->currentIndex()];
        // load the image in the pokemon label
        pokemon_img->setStyleSheet("image: url("
                                        ":img/" + pokemon.getUrlImg() +
                                        ");"
                                        "image-position: center center;"
                                        );
        pokemon_img->setText("");

        // adding type labels

        // for each type in the pokemon type list generate a custom colored label and assign it to the type layout
        foreach (QString type, pokemon.getTypes()) {
            QLabel *type_label = new QLabel();
            type_label->setText(type);
            type_label->setAlignment(Qt::AlignCenter); // align the text in the label

            type_label->setStyleSheet("QLabel { "
                                      "color: white;"
                                      "background-color : "
                                      + getTypeColor(_types_list, type) +
                                      ";"
                                      "padding: 5px;"
                                      "border-radius: 3px;"
                                      "font-size: 15pt;"
                                      "}");

            type_label->setFixedSize(100, 50);

            types_layout->addWidget(type_label);
        }

    }
}

void MainWindow::disablePokemonFromComboBox(QComboBox* pokemon1, QComboBox* pokemon2)
{
    // disable Pokemon in a combobox if already selected in the other one

    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(pokemon1->model());
    Q_ASSERT(model != nullptr);

    // enable all pokemons in pokemon1 combobox
    for(int i = 0; i < pokemon1->count(); i++) {
        QStandardItem *item = model->item(i);
        item->setEnabled(true);
    }

    // disable the pokemon2 in pokemon1 combobox if selected
    if(pokemon2->currentIndex() != -1) {
        QStandardItem *item = model->item(pokemon2->currentIndex());
        item->setEnabled(false);
    }

}

void MainWindow::tabChanged(int index)
{
    QStatusBar *status_bar = ui->statusbar;

    // hide the statusbar iwhen going to the compare tab
    if(index == 0) {
        status_bar->show();
    }
    else {
        status_bar->hide();
    }

}

void MainWindow::addPokemonToTable(QStringList headers)
{
    // get the table from ui
    QTableWidget *table = ui->pokemonTable;

    // create a bold font
    QFont bold_font;
    bold_font.setBold(true);

    // disable any type of focus, edit and selection, also enable sorting by header
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSortingEnabled(true);

    //  set the number of rows and columns
    table->setRowCount(_pokedex.size());
    table->setColumnCount(headers.size()-1);

    headers.removeAt(0); // removing the url_img part from the headers list
    table->setHorizontalHeaderLabels(headers); // setting the headers list on the table

    // hide the vertical headers
    table->verticalHeader()->hide();
    // make the horizontal headers bold by assigning the font
    table->horizontalHeader()->setFont(bold_font);

    // load pokemon informations in every row of the table
    unsigned int row_count = 0;

    foreach(Pokemon pokemon, _pokedex) {

        // loading number
        QTableWidgetItem *number = new QTableWidgetItem;
        // this puts an int in the table cell so that it can sort by number
        number->setData(Qt::EditRole, pokemon.getNumber());
        table->setItem(row_count, 0 ,number);

        // loading name
        QTableWidgetItem *name = new QTableWidgetItem;
        // this instead puts a string, so it'll sort by character
        name->setText(pokemon.getName());
        table->setItem(row_count, 1 ,name);

        // adding custom colored labels to the types cell
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
                                      + getTypeColor(_types_list, type) +
                                      ";"
                                      "padding: 5px;"
                                      "border-radius: 3px;"
                                      "}");

            types_layout->addWidget(type_label);
        }

        colorTableWidget->setLayout(types_layout);
        table->setCellWidget(row_count, 2 ,colorTableWidget);

        // loading the rest of the pokemon datas
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

    // aligning all numbers to the right, centering the type labels and setting the total and name values as bold
    // also setting the name writes as blue

    for(int i = 0; i < table->rowCount(); ++i) {

        table->item(i, 0)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        table->item(i, 1)->setFont(bold_font);
        table->item(i, 1)->setForeground(QBrush(QColor("#1570c3")));
        table->item(i, 2)->setData(Qt::TextAlignmentRole, Qt::AlignCenter);
        table->item(i, 3)->setFont(bold_font);

        for(int j = 3; j < table->columnCount(); ++j) {
            table->item(i,j)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }
    }

    // resizing row height to context so that that labels will be displayed correctly
    table->resizeRowsToContents();

    // make the table autoresize by stretching horizontally
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


    // showing the numbers of pokemon loaded in the statusbar
    ui->statusbar->showMessage(QStringLiteral("%1 pokemon shown.").arg(_pokedex.size()));

    // connect a slot to the clicked signal of the table
    connect(table, SIGNAL(cellClicked(int,int)), this, SLOT(onTableClicked(int,int)));


}

void MainWindow::onTableClicked(int row, int column)
{
    QString cell_text = ui->pokemonTable->item(row, column)->text();

    QComboBox* combobox = ui->pokemon1_combobox;
    if(combobox->findText(cell_text) != -1) { // if the text of the cell clicked is present in the combobox
        ui->tabWidget->setCurrentIndex(1); // change tab
        combobox->setCurrentIndex(combobox->findText(cell_text)); // set the combobox to the pokemon clicked
        ui->pokemon2_combobox->setCurrentIndex(-1); // set the other combobox to none
        setComparisonData(); // show data of the clicked pokemon
    }
}

QList<QStringList> MainWindow::parsePokedexCsv()
{

    QString pokedex_csv_path = ":/csv/pokedex.csv"; // path of the pokedex csv resource

    QFile file(pokedex_csv_path); // create a QFile from the csv file at the PokedexCSVPath
    QStringList word_list; // the list of string where i'm gonna save the csv lines

    QTextStream stream(&file); // open a new stream from the QFile

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) { // if the file opens correctly
        while(!stream.atEnd()) { // until the file textstream doesn't reach its end
            QString line = stream.readLine(); // take a line as a string and go to the next
            word_list.append(line); // append the line to the word_list created before
        }
    }
    else { // print info to debug console if it can't open the file
        qDebug() << "File exists: " << file.exists();
        qDebug() << "Error string: " << file.errorString();
        qDebug() << "QFileDevice::FileError value: " << file.error() <<
                    " - see: https://doc.qt.io/qt-5/qfiledevice.html#FileError-enum";
    }

    QStringList csv_row;
    QList<QStringList> pokemon_infos;

    // changing type separator from "," to "|" if the pokemon have multiple types
    for(QString& row: word_list) {
        csv_row = row.split('"'); // split the row using '"' as separator
        if(csv_row.size() > 1) { // this occurs only if the pokemon have multiple types and the line isn't the headers one
            csv_row[1].replace(",", "|");
            csv_row[1] = csv_row[1].simplified().remove(" "); // trim the types string from any whitespaces
        }
        row = csv_row.join(""); // rejoin the splitted row with the types substring edited and with no quotes ("")
    }

    foreach(QString row, word_list) {
        csv_row = row.split(","); // split it into a list of string using "," as a separator
        pokemon_infos.append(csv_row); // append the list to a list of QStringList
    }

    return pokemon_infos;
}

QList<Pokemon> MainWindow::getPokemonList(QList<QStringList> parsed_file)
{
    QList<Pokemon> pokemon_list;

    // generating a pokemon for every QStringList in parsed_file and appending it to pokemon_list
    foreach(QStringList pokemon_stats, parsed_file) {
        pokemon_list.append(createPokemon(pokemon_stats));
    }

    // printing the list of pokemon objects (autoconverted to a single QString)
    foreach(Pokemon pokemon, pokemon_list) {
        qDebug() << pokemon;
    }

    return pokemon_list;
}

Pokemon MainWindow::createPokemon(QStringList pokemon_stats)
{
    // create a Pokemon object from a QStringList
    QString url_img = pokemon_stats[0];
    unsigned int number = pokemon_stats[1].toInt();
    QString name = pokemon_stats[2];
    QStringList types = pokemon_stats[3].split("|"); // gets a QStringList splitting using "|" as separator
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

QStringList MainWindow::getTypesList() {
    QStringList types_list;

    foreach (Pokemon pokemon, _pokedex) {
        foreach(QString type, pokemon.getTypes())
            if(!types_list.contains(type)) // save a type only if not already present in the list
                types_list.append(type);
    }

    return types_list;
}

void MainWindow::addTypeFilters(QStringList types)
{
    // generate a checkbox for every type, connect it to the filter slot and add it to the filters layout
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
    switch(types.indexOf(type)) { // 0 corresponds to the first element of the QStringList, so on
        case 0: return "#79BF53"; // grass
            break;
        case 1: return "#984E94"; // poison
            break;
        case 2: return "#E97F34"; // fire
            break;
        case 3: return "#A890F0"; // flying
            break;
        case 4: return "#754EC7"; // dragon
            break;
        case 5: return "#6890F0"; // water
            break;
        case 6: return "#A8B820"; // bug
            break;
        case 7: return "#A8A878"; // normal
            break;
        case 8: return "#705848"; // dark
            break;
        case 9: return "#F8D030"; // electric
            break;
        case 10: return "#F85888"; // psychic
            break;
        case 11: return "#E0C068"; // ground
            break;
        case 12: return "#98D8D8"; // ice
            break;
        case 13: return "#B8B8D0"; // steel
            break;
        case 14: return "#F0B6BC"; // fairy
            break;
        case 15: return "#C03028"; // fighting
            break;
        case 16: return "#B8A038"; // rock
            break;
        case 17: return "#705898"; // ghost
            break;
        // in case other types are present in a newer version of the csv file
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

    // add a placeholder for the combobox and set it to no option (-1)

    pokemon1->setCurrentIndex(-1);
    pokemon1->setEditable(true); // allows the user write in the combobox
    pokemon1->setInsertPolicy(QComboBox::NoInsert); // with this the user can't insert new entries he writes in the combobox
    // pokemon1->setPlaceholderText(QStringLiteral("--Select a Pokemon--"));
    // need to use this and access the QLineEdit inside the combobox because the combobox is set as editable
    pokemon1->lineEdit()->setPlaceholderText(QStringLiteral("--Select a Pokemon--"));


    pokemon2->setCurrentIndex(-1);
    pokemon2->setEditable(true);
    pokemon1->setInsertPolicy(QComboBox::NoInsert);
    pokemon2->lineEdit()->setPlaceholderText(QStringLiteral("--Select a Pokemon--"));
}

void MainWindow::clearLayout(QLayout* layout)
{
    // iterate over all items in a layout and delete them if they're widgets
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

    // create radial and angular axis and add them to the graph

    // the line (angular) axis
    QCategoryAxis *angularAxis = new QCategoryAxis;
    angularAxis->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    angularAxis->setRange(0, 360);
    angularAxis->append("Total", 0);
    angularAxis->append("Speed", 1*(360/7));
    angularAxis->append("Sp. Def", 2*(360/7));
    angularAxis->append("Sp. Atk", 3*(360/7));
    angularAxis->append("Defense", 4*(360/7));
    angularAxis->append("Attack", 5*(360/7));
    angularAxis->append("HP", 6*(360/7));

    // the circular (radial) axis
    QValueAxis *radialAxis = new QValueAxis();
    radialAxis->setTickCount(11); // tick on the axis, 0-10-20-30-...-100
    radialAxis->setLabelFormat("%d");
    radialAxis->setRange(0, 100); // the range of the values

    chart->addAxis(angularAxis, QPolarChart::PolarOrientationAngular);
    chart->addAxis(radialAxis, QPolarChart::PolarOrientationRadial);

    // add the chartview to the layout
    ui->chart_layout->addWidget(chartView);

    return chart;

}

double MainWindow::getPercentageValue(unsigned int value, unsigned int max_value)
{
    // get the value in percentage compared to the max value
    return static_cast<double>(value)/max_value*100;
}

void MainWindow::updateSeries(QPolarChart* chart, Pokemon pokemon1, Pokemon pokemon2)
{
    // get max stats wrapped in a fake Pokemon
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

    Pokemon max("", 0, "", {}, max_total, max_hp, max_atk, max_def, max_sp_atk, max_sp_def, max_speed);

    // create a new lineseries for both the pokemons
    QLineSeries *series1 = generateLineSeries(pokemon1, max);
    setSeriesColor(series1, pokemon1, 60);

    QLineSeries *series2 = generateLineSeries(pokemon2, max);
    setSeriesColor(series2, pokemon2, 120);

    // add the new series to the chart
    chart->addSeries(series1);
    chart->addSeries(series2);

    // get the axis from the chart and attach them to the series
    const QList<QAbstractAxis *> axisList = chart->axes();

    for (QAbstractAxis *axis : axisList) {
        series1->attachAxis(axis);
        series2->attachAxis(axis);
    }

}

void MainWindow::setSeriesColor(QLineSeries* series, Pokemon pokemon, int brightness)
{
    // set the width of the line
    QPen pen = series->pen();
    pen.setWidth(2);
    series->setPen(pen);
    // set the color as the pokemon first-type color with brightness value
    QString color_hex(getTypeColor(_types_list, pokemon.getTypes().at(0)));
    if(color_hex == "white; border: 1px solid grey; color: black;")
        color_hex = "#000"; // if unknown type use black
    QColor *color = new QColor(color_hex);
    series->setColor(color->lighter(brightness).name());
}

QLineSeries *MainWindow::generateLineSeries(Pokemon pokemon, Pokemon max)
{
    QLineSeries *series = new QLineSeries();
    series->append(0, getPercentageValue(pokemon.getTotal(), max.getTotal()));
    series->append(1*(360/7), getPercentageValue(pokemon.getSpeed(), max.getSpeed()));
    series->append(2*(360/7), getPercentageValue(pokemon.getSpDef(), max.getSpDef()));
    series->append(3*(360/7), getPercentageValue(pokemon.getSpAtk(), max.getSpAtk()));
    series->append(4*(360/7), getPercentageValue(pokemon.getDef(), max.getDef()));
    series->append(5*(360/7), getPercentageValue(pokemon.getAtk(), max.getAtk()));
    series->append(6*(360/7), getPercentageValue(pokemon.getHp(), max.getHp()));
    series->append(360, getPercentageValue(pokemon.getTotal(), max.getTotal()));
    series->setName(pokemon.getName());

    return series;
}
