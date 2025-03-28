#pragma once

#include "lc_application.h"
#include "lc_shortcuts.h"
#include "lc_category.h"

struct lcPreferencesDialogOptions
{
	lcPreferences Preferences;

	QString LibraryPath;
	QString ColorConfigPath;
	QString MinifigSettingsPath;
	QString POVRayPath;
	QString LGEOPath;
	QString DefaultAuthor;
	QString Language;
	int CheckForUpdates;

	int AASamples;
	lcStudStyle StudStyle;

	std::vector<lcLibraryCategory> Categories;
	bool CategoriesModified;
	bool CategoriesDefault;

/*** LPub3D Mod - parts load order ***/
	bool HasUnofficialParts;
/*** LPub3D Mod - ***/

	lcKeyboardShortcuts KeyboardShortcuts;
	bool KeyboardShortcutsModified;
	bool KeyboardShortcutsDefault;

	lcMouseShortcuts MouseShortcuts;
	bool MouseShortcutsModified;
	bool MouseShortcutsDefault;
};

namespace Ui
{
class lcQPreferencesDialog;
}

class lcQPreferencesDialog : public QDialog
{
	Q_OBJECT

public:
/*** LPub3D Mod - Load Preferences Dialog ***/
	lcQPreferencesDialog(QWidget* Parent);
/*** LPub3D Mod end ***/
	~lcQPreferencesDialog();

	lcPreferencesDialogOptions* mOptions;

	enum
	{
		CategoryRole = Qt::UserRole
	};

/*** LPub3D Mod - Load Preferences Dialog ***/
	void setOptions(lcPreferencesDialogOptions* Options);
/*** LPub3D Mod end ***/

	bool eventFilter(QObject* Object, QEvent* Event) override;

public slots:
	void accept() override;
	void on_partsLibraryBrowse_clicked();
	void on_partsArchiveBrowse_clicked();
	void on_ColorConfigBrowseButton_clicked();
	void on_MinifigSettingsBrowseButton_clicked();
	void on_povrayExecutableBrowse_clicked();
	void on_lgeoPathBrowse_clicked();
	void on_ColorTheme_currentIndexChanged(int Index);
	void ColorButtonClicked();
/*** LPub3D Mod - Set default colour ***/
	void ResetFadeHighlightColor();
/*** LPub3D Mod end ***/
	void AutomateEdgeColor();
	void on_AutomateEdgeColor_toggled();
/*** LPub3D Mod - disabled, there is not BlenderAddonSettingsButton ***/
/*  void on_BlenderAddonSettingsButton_clicked(); */
/*** LPub3D Mod end ***/
/*** LPub3D Mod - line width max granularity ***/
	void LineWidthMaxGranularity();
/*** LPub3D Mod end ***/
	void on_studStyleCombo_currentIndexChanged(int index);
	void on_antiAliasing_toggled();
	void on_edgeLines_toggled();
	void on_ConditionalLinesCheckBox_toggled();
	void on_LineWidthSlider_valueChanged();
	void on_MeshLODSlider_valueChanged();
/*** LPub3D Mod - lpub fade highlight ***/
	void on_LPubFadeHighlight_toggled();
/*** LPub3D Mod end ***/
	void on_FadeSteps_toggled();
	void on_HighlightNewParts_toggled();
	void on_gridStuds_toggled();
	void on_gridLines_toggled();
	void on_ViewSphereSizeCombo_currentIndexChanged(int Index);
/*** LPub3D Mod - preview widget for LPub3D ***/
//	void on_PreviewViewSphereSizeCombo_currentIndexChanged(int Index);
/*** LPub3D Mod end ***/
	void updateParts();
	void on_newCategory_clicked();
	void on_editCategory_clicked();
	void on_deleteCategory_clicked();
	void on_importCategories_clicked();
	void on_exportCategories_clicked();
	void on_resetCategories_clicked();
	void on_shortcutAssign_clicked();
	void on_shortcutRemove_clicked();
	void on_shortcutsImport_clicked();
	void on_shortcutsExport_clicked();
	void on_shortcutsReset_clicked();
	void commandChanged(QTreeWidgetItem *current);
	void on_KeyboardFilterEdit_textEdited(const QString& Text);
	void on_mouseAssign_clicked();
	void on_mouseRemove_clicked();
	void on_MouseImportButton_clicked();
	void on_MouseExportButton_clicked();
	void on_mouseReset_clicked();
	void MouseTreeItemChanged(QTreeWidgetItem* Current);

/*** LPub3D Mod - preview widget ***/
	void on_PreviewSizeCombo_currentIndexChanged(int Index);
	void on_PreviewViewSphereSizeCombo_currentIndexChanged(int Index);
	void on_PreviewPositionCombo_currentIndexChanged(int Index);
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Native Renderer settings ***/
	void on_ViewpointsCombo_currentIndexChanged(int index);
	void on_ProjectionCombo_currentIndexChanged(int index);
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Reset theme colors ***/
	void on_ResetColorsButton_clicked();
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Update Default Camera ***/
	void on_cameraDefaultDistanceFactor_valueChanged(double value);
	void on_cameraDefaultPosition_valueChanged(double value);
	void cameraPropertyReset();
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Enable background colour buttons ***/
	void on_BackgroundSolidRadio_toggled(bool checked);
	void on_BackgroundGradientRadio_toggled(bool checked);
/*** LPub3D Mod end ***/

private:
	Ui::lcQPreferencesDialog *ui;

	void updateCategories();
	void updateCommandList();
	void UpdateMouseTree();
	void UpdateMouseTreeItem(int ItemIndex);
	void setShortcutModified(QTreeWidgetItem *treeItem, bool modified);

	float mLineWidthRange[2];
	float mLineWidthGranularity;
	static constexpr float mMeshLODMultiplier = 25.0f;
/*** LPub3D Mod - Load Preferences Dialog ***/
	bool mSetOptions;
/*** LPub3D Mod end ***/
};
