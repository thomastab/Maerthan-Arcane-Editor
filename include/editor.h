#include <QtWidgets/QMainWindow>
#include <QtWidgets/QComboBox>
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <element.h>
#include "ui_editor.h"
#include <vector>
#include <modifierWidget.h>

class Editor : public QMainWindow
{
	Q_OBJECT

public:
	Editor(QWidget* parent = Q_NULLPTR);

private:
	Ui::EditorClass _ui;
	std::vector<Element> _elements;
	std::vector<ModifierWidget*> _modifierWidgets;
	int _overload = 99;
	int _masteryLevel = 0;

	void loadConfig(std::string configPath);
	void clearLayout(QLayout* layout);
	void updateOverload();

private slots:
	void onCostChanged();
	void on_comboBoxElement_currentIndexChanged(int index);
	void on_comboBoxMastery_currentIndexChanged(int index);
};
