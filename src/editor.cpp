#include <editor.h>

Editor::Editor(QWidget* parent)
    : QMainWindow(parent)
{
    _ui.setupUi(this);
    loadConfig("settings.json");

    QComboBox elementSelection;
    for (int i = 0; i < _elements.size(); i++) {
        _ui.comboBoxElement->addItem(QString::fromStdString(_elements[i].getName()));
    }
}

void Editor::loadConfig(std::string configPath) {
    using json = nlohmann::json;

    // Read json config
    json jsonSource;
    std::ifstream file;
    file.open(configPath);
    file >> jsonSource;

    // Loop over each element in json
    for (int elem = 0; elem < jsonSource["Elements"].size(); elem++) {

        Element tempElem;
        tempElem.setName(jsonSource["Elements"][elem].value("Name", "Unreadable"));
        tempElem.setOverloads(jsonSource["Elements"][elem]["Overloads"].get<std::vector<int>>());

        // Loop over each modifier in element
        for (int mod = 0; mod < jsonSource["Elements"][elem]["Modifiers"].size(); mod++) {

            Modifier modifier(jsonSource["Elements"][elem]["Modifiers"][mod].value("Name", "Unreadable"));

            // Loop over each mastery level (3 max)
            for (int mLevel = 0; mLevel < 3; mLevel++) {

                std::vector<int> attributes = jsonSource["Elements"][elem]["Modifiers"][mod]["Attribute"][mLevel].get<std::vector<int>>();
                std::vector<int> costs = jsonSource["Elements"][elem]["Modifiers"][mod]["Cost"][mLevel].get<std::vector<int>>();

                if (jsonSource["Elements"][elem]["Modifiers"][mod]["CostType"].get<std::string>() == "multiple") {
                    modifier.setCostOperator(std::multiplies<int>());
                }
                else {
                    modifier.setCostOperator(std::plus<int>());
                }

                modifier.addAttribute(mLevel, attributes);
                modifier.addCost(mLevel, costs);
            }
            tempElem.addModifier(modifier);
        }

        _elements.push_back(tempElem);
    }
}

void Editor::clearLayout(QLayout* layout) {
    QLayoutItem* item;
    while ((item = layout->takeAt(0))) {
        if (item->layout()) {
            clearLayout(item->layout());
            delete item->layout();
        }
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

void Editor::updateOverload() {
    int currentElementIndex = _ui.comboBoxElement->currentIndex();
    _overload = _elements[currentElementIndex].getOverloads()[_masteryLevel];
    _ui.lcdNumberOverload->display(QString::fromStdString(std::to_string(_overload)));
}

void Editor::onCostChanged() {
    int finalCost = 0;
    for (ModifierWidget* widget : _modifierWidgets) {
        finalCost = widget->getModifier().getCostOperator()(finalCost, widget->getCurrentCost());
    }
    if (finalCost > _overload) {
        _ui.lcdNumberCost->setStyleSheet("color: rgb(255,150,150);");
    }
    else {
        _ui.lcdNumberCost->setStyleSheet("");
    }
    _ui.lcdNumberCost->display(QString::fromStdString(std::to_string(finalCost)));
}

void Editor::on_comboBoxElement_currentIndexChanged(int index) {
    clearLayout(_ui.modifiersLayout);
    _modifierWidgets.clear(); // WARNING, MEMORY LEAKS AS WE DON'T DELETE POINTERS

    int modifierNb = _elements[index].getModifiers().size();

    for (int i = 0; i < modifierNb; i++) {
        ModifierWidget* modifierWidget = new ModifierWidget(_elements[index].getModifiers()[i], _masteryLevel);
        _modifierWidgets.push_back(modifierWidget);
        connect(modifierWidget, SIGNAL(costChanged()), this, SLOT(onCostChanged()));
        connect(_ui.comboBoxMastery, SIGNAL(currentIndexChanged(int)), modifierWidget, SLOT(onMasteryLevelChanged(int)));
        _ui.modifiersLayout->addWidget(modifierWidget);
    }
    onCostChanged();
    updateOverload();
}

void Editor::on_comboBoxMastery_currentIndexChanged(int index) {
    _masteryLevel = index;
    updateOverload();
}