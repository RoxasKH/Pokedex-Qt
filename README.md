# Pokedex Qt
> A simple Pokedex application written with the help of the C++ Qt framework.

![Pokedex Pokedex Showcase](screenshots/pokedex_showcase.png?raw=true "Showcase of the pokedex screen")
![Pokedex Compare Showcase](screenshots/pokedex_compare_showcase.png?raw=true "Showcase of the compare screen")

The program allows the user to easily check and search for informations about all the Pokemons available at the time of creation of the program, and compare their statistics.

The project was successfully compiled and tested on: 

- Windows 10 Home 64-bit 22H2 build 19045.2486
- Linux Mint 20.3 64-bit

Using:

- Qt Creator 9.0.1 based on Qt 6.4.0

_Note: Pokemon© is a registered trademark owned by Nintendo, Gamefreak, and Creatures Inc., but this is just a simple application project made for fun. I get no profit at all from this, and the code is open-source. For any problems just contact me, you can find contact information in my github profile page._

## Table of Contents

- [Chosen languages for the implementation](#chosen-languages-for-the-implementation)
	- [Requirements](#requirements)
	- [Running the app](#running-the-app)
- [GUI](#gui)
	- [Pokedex screen](#Pokedex-screen)
	- [Comparison screen](#comparison-screen)
- [Some details and resources](#some-details-and-resources)
- [Updating the program to new sets of Pokemons](#updating-the-program-to-new-sets-of-pokemons)
- [Deployment options](#deployment-options)
	- [windeployqt](#windeployqt)
	- [macdeployqt](#macdeployqt)
	- [linuxdeployqt](#linuxdeployqt)
- [Known issues](#known-issues)


## Chosen languages for the implementation

For the application the following languages were used:

- C++ with [Qt framework](https://www.qt.io/): used for defining both the logic and the frontend of the app, with the help of the [Qt Creator cross-platform IDE](https://www.qt.io/product/development-tools)

Additional Qt libraries:

- [Qt Charts](https://doc.qt.io/qt-6/qtcharts-index.html): used to generate the QPolarChart for the statistics comparison

### Requirements

So, to compile and run the app, you will need to install the Qt Creator IDE on your system. 
When installing Qt Creator, Qt 6 or later is needed, as [Qt Charts no longer has a custom QtCharts namespace](https://doc.qt.io/qt-6/qtcharts-changes-qt6.html). If on Windows, make also sure to select the MinGW based build of Qt.
In the Qt Maintenance Tool, under the Qt Additional Libraries voice, select Qt Charts as well, as it's needed for the program to run correctly.

_Note: program may works on older Qt versions, but you'll have to add the `using namespace QtCharts;` instruction to it._

### Running the app

Well, if you just want to run the app easily you can go the Release section of the repository on Github and pickup the correct release build for your OS (at the moment, unluckily only a Windows build is present. I may add a Linux one in the future tho, so keep an eye opened).

If instead your OS is not listed or you want to compile and run the app on your system, to be able to look or even edit part of the code, follow these steps:

1. Download the project folder
2. Open Qt Creator
3. From Qt Creator, locate and open the project by opening the `Pokedex.pro` file (you may have to reconfigure it, but it's usually as easy as clicking a button)
4. Once loaded, select the mode you wanna compile and run the project from the computer icon (`Release` or `Debug`; note that release mode will remove any debug printing from the console)
5. Build the project clicking on the hammer icon
6. Finally run the program through the green play button icon

Running the app, the application window will open and you can start trying the program functionalities.

## GUI

The interface consists of a main window in which to view a table with the list of Pokemons with their information attached to each of them, and another window where it's possible to compare characteristics between two Pokemons.

The whole GUI is dinamically resizable.

### Pokedex Screen

![Pokedex Filtering](screenshots/pokedex_filtering.png?raw=true "Pokedex screen with filtering on")

While in the main screen, you can check and see the whole Pokemon list in the main table, with their informations such as their number, name, the list of types, attack, defense, and so on.

You can also sort in ascending/descending order the Pokemon list by a specific statistic, through clicking on the corresponding column header in the table. Keep in mind that due to its nature, sorting them by type is not possible. Use type filtering instead.

As scrolling through more than a thousand Pokemons may be annoying to find specific informations, some filtering options are provided in the filtering menu on the right.

The search bar allows you to search for a Pokemon by writing its name, or its number in the Pokedex. Note that, while searching a Pokemon by number, due to how the hybrid filtering logic works, some Pokemons having numbers in their name (ex. Porygon22 or Zygarde forms) may appear as well.

Under the search bar, you can find the type filtering checkboxes. Ticking a filter checkbox will make all the Pokemons that have at least one of the selected types in their types list appear. 
For a more precise type filtering, you can check the _Selected Types Only_ checkbox; that will make so that only Pokemons that have those specific chosen types as their type will be shown.

After filtering, the number of Pokemons shown resulted by the filtering is shown in the status bar on the bottom of the app.

From this tab, you can click on the Compare tab to move to it. You can also click on the table cell containing a Pokemon name to move to the Compare tab and have it set as the first Pokemon for the comparison.

### Comparison Screen

![Pokemon Comparison](screenshots/pokedex_comparison_from_table.png?raw=true "Comparison screen")

In the comparison tab, you can compare two Pokemons, seeing their image, types and a polar chart containing the statistics for both of them.

You can set the Pokemon you wanna compare through the comboboxes at the top. The comboboxes are made editable so that you can start writing a Pokemon name and get suggestions from the combobox entries you can select by pressing enter. That way, you don't have to deal with searching the Pokemon you need in the 1000+ comboboxes entries.

Once a Pokemon is selected, its image and types list are loaded, and its statistics are attached to the chart.

When a Pokemon is selected on a combobox the corresponding item in the other one gets disabled, so that you cannot (theoretically, check [Known issues](#known-issues) section for more informations about it) compare a Pokemon with itself.

## Some details and resources

The whole code is completely commented for a better understanding of it.

The program relies on a custom Pokemon class, which basically acts as a container for the Pokemon informations.

Images type labels are styled using Qt StyleSheets.

The chart used is a polar chart from the Qt Charts library, one of the most intuitive and used for these types of comparisons.
Few resources are online about it, you can check the [Qt Docs example](https://doc.qt.io/qt-6/qtcharts-polarchart-example.html) for a better understanding of it, and [this stackoverflow post](https://stackoverflow.com/questions/50000071/qt-qpolarchart-with-categorial-angular-axis) to know how to add a category name to the axis.

Or well, as i had myself some problems figuring it out, you can also directly check my code. As i said it's well commented, but for how the graph needs to dinamically change, its code is a bit fragmented over different methods. Still, it may help understanding how the QPolarChart works.

The LineSeries color as you can see is dynamically set based on the Pokemon first type color. As this way comparing two Pokemons of the same type would have been a mess, different brightness levels are set for the QColor of the first and second Pokemon.

## Updating the program to new sets of Pokemons

Pokemons and Pokemon types are loaded dynamically from the `pokedex.csv` file. Not only that, graph values are also calculated as a percentage of the max stat value present in the `.csv` file.

So updating resources (the `pokedex.csv` file and the Pokemon images in the images folder) and the project resource file, should be enough for the app to work on an updated Pokemon sets.

That said, setting the proper color for a type will be more difficult as it involves editing part of the code, and as there's no switch-case construct working on strings in C++, you'll need to check the obtained types list and edit the function that gets the correct type colors accordingly.

But don't worry, in the meantime types will be correctly loaded, colors are the only thing that may be off depending on how the types are taken from the Pokemon list. If only Pokemons with a higher number than the last are appended to the Pokedex, old types will be loaded correctly, and possible new types will be loaded in a black and white label (and their line in the chart will be black as well).
The program functionalities although, won't break.

## Deployment options

A list of tools that may be useful when deploying and testing a similar Qt application.

### windeployqt

The internal Qt tool windeployqt was used to deploy the application for Windows. The tool automatically adds the necessary library for your compiled executable to work on Windows.

You chan check the [Qt Wiki](https://wiki.qt.io/Deploy_an_Application_on_Windows) and [Docs](https://doc.qt.io/qt-6/windows-deployment.html) to understand how it works.

Basically, you compile your app in Release mode, then you go to the build folder in your Qt Creator project folder, and copy the compiled `.exe` file to a newly created folder you can put where you want.

You then open the command prompt that was installed along with Qt Creator, and run the command:

`windeployqt.exe {path-to-your-compiled-exe}\{name-of the-exe-file}.exe`

where you need to substitute:

- `{path-to-your-compiled-exe}` with the path of he folder you put the .exe in
- `{name-of the-exe-file}` with the name of your compiled .exe file

And the windeployqt tool will copy all the needed libraries for your app to work in the folder.

### macdeployqt

A similar internal tool is present for macOS deployment as well, called macdeployqt. This generates working `.app` and `.dmg` packages.
You can check about it more on the [Qt Docs](https://doc.qt.io/qt-5/macos-deployment.html#macdeploy).

### linuxdeployqt

A 3rd party linux deployment tool called linuxdeployqt is available on github. It's based on the official mac deployment tool, but generates AppDirs and AppImages instead.

https://github.com/probonopd/linuxdeployqt

## Known issues

Unluckily, the application isn't flawless.

Apparently, setting the combobox items as disabled through `item->setEnabled(true);` will disable the entry from the combobox list, and if the combobox is set as editable, it will remove the entry from the hints you can get while writing, but writing the full name of the Pokemon entry and pressing enter will load the Pokemon anyway. So it's actually possible to compare a Pokemon with itself.

If you want to keep the comboboxes as editable, this can be easily fixed by creating a method that will remove completely the entry from the combobox when selected on the other one.

As the Qt Docs (and internet in general) has really few documentation about editable QComboBoxes, it's difficult to know what exactly is happening, and as i don't really like the fix i wrote before, i'm keeping it like this.

![Pokemon Comparison Issue](screenshots/comparison_issue.png?raw=true "Comparison issue")

There's also another issue with the comboboxes, which this time i don't think it's even code dependent, but seems to be more like of a Qt bug. Even if the two comboboxes are both initialized the same way, set as editable with insertpolicy set as `NoInsert` so that writing in the combobox lineEdit strings not present in the combobox entries and pressing enter should do nothing, if you do so for the combobox of the second Pokemon (the one on the right), the program crashes.
I have no idea what would be causing this, but as i said i suspect is more of a framework bug than anything.