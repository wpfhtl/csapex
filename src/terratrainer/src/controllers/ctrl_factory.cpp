#include "ctrl_factory.h"
#include <gui/terra_trainer_window.h>
#include <gui/terra_sub_window.h>
/// UI
#include <ui_terra_toolbar.h>
#include <ui_terra_trainer_window.h>
#include <ui_terra_classes_window.h>
#include <ui_terra_preferences.h>


/// CONTROLLERS
#include "controller.hpp"
#include <controllers/ctrl_cmpcore_bridge.h>
#include <controllers/ctrl_main_menu.h>
#include <controllers/ctrl_map_view.h>
#include <controllers/ctrl_toolpanel.h>
#include <controllers/ctrl_class_edit.h>
#include <controllers/ctrl_preferences.h>

void CtrlFactory::produdeBridgeController(TerraTrainerWindow *mainWindow)
{
    if(mainWindow->core_.get() == NULL) {
        std::cerr << "Compuatation Core not yet initialized, therefore cancelling Controller Bridge initialization!";
        return;
    }

    CMPCoreBridge *ctrl = new CMPCoreBridge(mainWindow->core_);

    Controller::IDPtr entry(Controller::Bridge, Controller::Ptr(ctrl));
    mainWindow->controllers_.insert(entry);
}

void CtrlFactory::produceMapViewController(TerraTrainerWindow *mainWindow)
{
    Ui::TerraTrainerWindow *ui = mainWindow->ui_;
    CMPCoreBridge::Ptr  br = Controller::to<CMPCoreBridge>(mainWindow->controllers_[Controller::Bridge]);

    if(br == NULL) {
        std::cerr << "Bridge Controller not yet initialzed, cancelling Map View Controller init!" << std::endl;
        return;
    }

    CtrlMapView *ctrl = new CtrlMapView(br);
    ctrl->setupUI(ui);
    QObject::connect(br.get(), SIGNAL(imageLoaded()),         ctrl, SLOT(imageUpdate()),         Qt::QueuedConnection);
    QObject::connect(br.get(), SIGNAL(classRemoved(int)),     ctrl, SLOT(classRemoved(int)),     Qt::QueuedConnection);
    QObject::connect(br.get(), SIGNAL(classUpdated(int,int)), ctrl, SLOT(classUpdated(int,int)), Qt::QueuedConnection);
    QObject::connect(br.get(), SIGNAL(colorUpdate(int)),      ctrl, SLOT(colorUpdate(int)),      Qt::QueuedConnection);
    QObject::connect(br.get(), SIGNAL(computeFinished()),     ctrl, SLOT(computeFinished()),     Qt::QueuedConnection);
    QObject::connect(br.get(), SIGNAL(computeGridFinished()), ctrl, SLOT(computeGridFinished()), Qt::QueuedConnection);
    QObject::connect(br.get(), SIGNAL(computeQuadFinished()), ctrl, SLOT(computeQuadFinished()), Qt::QueuedConnection);
    QObject::connect(ctrl,     SIGNAL(computeROIS()),         br.get(), SLOT(computeROIS()),         Qt::QueuedConnection);
    QObject::connect(ctrl,     SIGNAL(computeGRID()),         br.get(), SLOT(computeGRID()),         Qt::QueuedConnection);
    QObject::connect(ctrl,     SIGNAL(computeQUAD()),         br.get(), SLOT(computeQUAD()),         Qt::QueuedConnection);


    Controller::IDPtr entry(Controller::MapView, Controller::Ptr(ctrl));
    mainWindow->controllers_.insert(entry);
}

void CtrlFactory::produceMenuController(TerraTrainerWindow *mainWindow)
{
    Ui::TerraTrainerWindow *ui = mainWindow->ui_;
    CtrlMainWindow         *ctrl = new CtrlMainWindow(mainWindow);
    CMPCoreBridge::Ptr      br = Controller::to<CMPCoreBridge>(mainWindow->controllers_[Controller::Bridge]);
    CtrlMapView::Ptr        mv = Controller::to<CtrlMapView>(mainWindow->controllers_[Controller::MapView]);

    ctrl->setupUi(ui);

    if(mv == NULL){
        std::cerr << "Map View Controller not yet initialized, therefore cancelling Menu Controller initialization!" << std::endl;
        return;
    }

    QAction::connect(ui->action_LoadImage,              SIGNAL(triggered()), ctrl, SLOT(loadImage()));
    QAction::connect(ui->action_LoadClassifier,         SIGNAL(triggered()), ctrl, SLOT(loadClassifier()));
    QAction::connect(ui->action_SaveClassifierProject,  SIGNAL(triggered()), ctrl, SLOT(saveClassifierProject()));
    QAction::connect(ui->action_SaveClassifierRaw,      SIGNAL(triggered()), ctrl, SLOT(saveClassifierRaw()));
    QAction::connect(ui->action_ZoomIn,                 SIGNAL(triggered()), ctrl, SLOT(zoomIn()));
    QAction::connect(ui->action_ZoomOut,                SIGNAL(triggered()), ctrl, SLOT(zoomOut()));
    QAction::connect(ui->action_ZoomReset,              SIGNAL(triggered()), ctrl, SLOT(zoomReset()));
    QAction::connect(ui->action_SaveCrops,              SIGNAL(triggered()), ctrl, SLOT(saveCrops()));
    QAction::connect(ui->action_SaveROIs,               SIGNAL(triggered()), ctrl, SLOT(saveROIs()));
    QAction::connect(ui->action_SaveBatchTemplate,      SIGNAL(triggered()), ctrl, SLOT(saveBatchTemplate()));
    QAction::connect(ui->action_LoadSettings,           SIGNAL(triggered()), ctrl, SLOT(loadSettings()));
    QAction::connect(ui->action_SaveSettings,           SIGNAL(triggered()), ctrl, SLOT(saveSettings()));
    QAction::connect(ui->action_LoadRois,               SIGNAL(triggered()), ctrl, SLOT(loadROIs()));
    QObject::connect(mv.get(),                          SIGNAL(zoomUpdated(double)), ctrl, SLOT(zoomUpdate(double)));
    QObject::connect(ctrl,                              SIGNAL(zoom(double)), mv.get(), SLOT(zoom(double)));
    QObject::connect(ctrl,                              SIGNAL(loadIMAGE(QString)), br.get(), SLOT(loadIMAGE(QString)));

    Controller::IDPtr entry(Controller::Menu, Controller::Ptr(ctrl));
    mainWindow->controllers_.insert(entry);
}

void CtrlFactory::produceToolBarController(TerraTrainerWindow *mainWindow)
{
    Ui::ToolPanel      *tp = mainWindow->tool_panel_ui_;
    CMPCoreBridge::Ptr  br = Controller::to<CMPCoreBridge>(mainWindow->controllers_[Controller::Bridge]);
    CtrlMapView::Ptr    mv = Controller::to<CtrlMapView>(mainWindow->controllers_[Controller::MapView]);
    CtrlMainWindow::Ptr mm = Controller::to<CtrlMainWindow>(mainWindow->controllers_[Controller::Menu]);

    if(br == NULL) {
        std::cerr << "Bridge Controller not yet initialized, therefore cancelling ToolPanel Controller initialization!" << std::endl;
        return;
    }

    if(mv == NULL) {
        std::cerr << "Map View Controller not yet initialized, therefore cancelling ToolPanel Controller initialization!" << std::endl;
        return;
    }

    CtrlToolPanel *ctrl = new CtrlToolPanel(mainWindow->tool_panel_window_, br);
    ctrl->setupUI(tp);

    /// BUTTON CONNECTIONS
    QPushButton::connect(tp->zoomIn,    SIGNAL(clicked()),           ctrl,           SLOT(zoomIn()));
    QPushButton::connect(tp->zoomOut,   SIGNAL(clicked()),           ctrl,           SLOT(zoomOut()));

    QPushButton::connect(tp->addBoxes,  SIGNAL(clicked()),                ctrl,      SLOT(buttonAdd()));
    QPushButton::connect(tp->movBoxes,  SIGNAL(clicked()),                ctrl,      SLOT(buttonMov()));
    QPushButton::connect(tp->delBoxes,  SIGNAL(clicked()),                ctrl,      SLOT(buttonDel()));
    QPushButton::connect(tp->selBoxes,  SIGNAL(clicked()),                ctrl,      SLOT(buttonSel()));

    /// computation
    QPushButton::connect(tp->compile,     SIGNAL(clicked()), ctrl,      SLOT(buttonCompute()));
    QPushButton::connect(tp->computeGrid, SIGNAL(clicked()), ctrl,      SLOT(buttonGrid()));
    QPushButton::connect(tp->computeTree, SIGNAL(clicked()), ctrl,      SLOT(buttonQuad()));
    QObject::connect(ctrl,                SIGNAL(compute()), mv.get(), SLOT(compute()));
    QObject::connect(ctrl,                SIGNAL(grid()),    mv.get(), SLOT(computeGrid()));
    QObject::connect(ctrl,                SIGNAL(quad()),    mv.get(), SLOT(computeQuad()));

    QComboBox::connect  (tp->classes,   SIGNAL(currentIndexChanged(int)), ctrl,      SLOT(classChanged(int)));

    QPushButton::connect(ctrl,          SIGNAL(uncheckAdd(bool)),    tp->addBoxes,   SLOT(setChecked(bool)));
    QPushButton::connect(ctrl,          SIGNAL(uncheckMov(bool)),    tp->movBoxes,   SLOT(setChecked(bool)));
    QPushButton::connect(ctrl,          SIGNAL(uncheckDel(bool)),    tp->delBoxes,   SLOT(setChecked(bool)));
    QPushButton::connect(ctrl,          SIGNAL(uncheckSel(bool)),    tp->selBoxes,   SLOT(setChecked(bool)));

    QObject::connect(br.get(),          SIGNAL(classAdded(int)),      ctrl,           SLOT(classAdded(int)),       Qt::QueuedConnection);
    QObject::connect(br.get(),          SIGNAL(classRemoved(int)),    ctrl,           SLOT(classRemoved(int)),     Qt::QueuedConnection);
    QObject::connect(br.get(),          SIGNAL(classUpdated(int,int)),ctrl,           SLOT(classUpdated(int,int)), Qt::QueuedConnection);
    QObject::connect(br.get(),          SIGNAL(colorUpdate(int)),     ctrl,           SLOT(colorUpdate(int)),      Qt::QueuedConnection);
    QObject::connect(br.get(),          SIGNAL(imageLoaded()),        ctrl,           SLOT(image_loaded()),        Qt::QueuedConnection);
    QObject::connect(br.get(),          SIGNAL(classesCleared()),     ctrl,           SLOT(classesCleared()),      Qt::QueuedConnection);
    QObject::connect(tp->addBoxes,      SIGNAL(clicked()),           mv.get(),       SLOT(activateAdd()));
    QObject::connect(tp->movBoxes,      SIGNAL(clicked()),           mv.get(),       SLOT(activateMove()));
    QObject::connect(tp->delBoxes,      SIGNAL(clicked()),           mv.get(),       SLOT(activateDelete()));
    QObject::connect(tp->selBoxes,      SIGNAL(clicked()),           mv.get(),       SLOT(activateSelect()));
    QObject::connect(tp->trash,         SIGNAL(clicked()),           mv.get(),       SLOT(activateTrash()));
    QObject::connect(tp->selAll,        SIGNAL(clicked()),           mv.get(),       SLOT(selectAll()));
    QObject::connect(tp->deselAll,      SIGNAL(clicked()),           mv.get(),       SLOT(deselectAll()));

    QObject::connect(mm.get(),          SIGNAL(syncSettingsCore()),  ctrl,           SLOT(syncExtractorParams()));
    QObject::connect(mv.get(),          SIGNAL(zoomUpdated(double)), ctrl,           SLOT(zoomUpdate(double)));
    QObject::connect(ctrl,              SIGNAL(zoom(double)),        mv.get(),       SLOT(zoom(double)));
    QObject::connect(ctrl,              SIGNAL(classSelected(int)),  mv.get(),       SLOT(changeClass(int)));
    QDoubleSpinBox::connect(tp->zoomBox,SIGNAL(valueChanged(double)),mv.get(),       SLOT(zoom(double)));
    QDoubleSpinBox::connect(mv.get(),   SIGNAL(zoomUpdated(double)), tp->zoomBox,    SLOT(setValue(double)));
    QDoubleSpinBox::connect(tp->sizeBox,SIGNAL(valueChanged(double)),mv.get(),       SLOT(size(double)));
    QDoubleSpinBox::connect(mv.get(),   SIGNAL(sizeUpdated(double)), tp->sizeBox,    SLOT(setValue(double)));

    Controller::IDPtr entry(Controller::ToolPanel, Controller::Ptr(ctrl));
    mainWindow->controllers_.insert(entry);
}

void CtrlFactory::produceClassEdController(TerraTrainerWindow *mainWindow)
{
    Ui::TerraClasses    *tc = mainWindow->classes_ui_;
    CMPCoreBridge::Ptr   br = Controller::to<CMPCoreBridge>(mainWindow->controllers_[Controller::Bridge]);

    if(br == NULL) {
        std::cerr << "Bridge Controller not yet initialized, therefore cancelling ToolPanel Controller initialization!" << std::endl;
        return;
    }

    CtrlClassEdit           *ctrl = new CtrlClassEdit(mainWindow->classes_window_, br);
    ctrl->setupUI(mainWindow->classes_ui_);

    QLineEdit::connect(tc->className,  SIGNAL(textEdited(QString)), ctrl,       SLOT(editInfo(QString)));
    QLineEdit::connect(tc->classID,    SIGNAL(textEdited(QString)), ctrl,       SLOT(editId(QString)));

    QComboBox::connect(tc->classColor, SIGNAL(currentIndexChanged(int)), ctrl, SLOT(colorIndex(int)));

    QPushButton::connect(tc->addClass, SIGNAL(clicked()),    ctrl,              SLOT(accept()));
    QPushButton::connect(tc->delClass, SIGNAL(clicked()),    ctrl,              SLOT(remove()));
    QPushButton::connect(ctrl,         SIGNAL(enableDel(bool)), tc->delClass,   SLOT(setEnabled(bool)));
    QPushButton::connect(ctrl,         SIGNAL(enableAdd(bool)), tc->addClass,   SLOT(setEnabled(bool)));

    QTableWidget::connect(tc->tableClasses, SIGNAL(cellClicked(int,int)), ctrl, SLOT(cellClicked(int,int)));

    Controller::IDPtr entry(Controller::Class, Controller::Ptr(ctrl));
    mainWindow->controllers_.insert(entry);
}

void CtrlFactory::produceSettingController(TerraTrainerWindow *mainWindow)
{
    Ui::TerraPreferences    *tf = mainWindow->preferences_ui_;
    CMPCoreBridge::Ptr   br = Controller::to<CMPCoreBridge>(mainWindow->controllers_[Controller::Bridge]);
    CtrlToolPanel::Ptr   tp = Controller::to<CtrlToolPanel>(mainWindow->controllers_[Controller::ToolPanel]);

    if(br == NULL) {
        std::cerr << "Bridge Controller not yet initialized, therefore cancelling Settings Controller initialization!" << std::endl;
        return;
    }

    if(tp  == NULL) {
        std::cerr << "Toolpanel Controller not yet initialized, cancelling Settings Controller initialization!" << std::endl;
        return;
    }

    CtrlPreferences         *ctrl = new CtrlPreferences(br);
    ctrl->setupUI(tf);

    /// ORB
    QCheckBox::connect(tf->orbBox,              SIGNAL(clicked(bool)),            ctrl, SLOT(orbOppChanged(bool)));
    QCheckBox::connect(tf->checkBox_colExtOrb,  SIGNAL(clicked(bool)),            ctrl, SLOT(orbColorExtChanged(bool)));
    QSpinBox::connect (tf->spinBox_levelOrb,    SIGNAL(valueChanged(int)),        ctrl, SLOT(orbLevelChanged(int)));
    QSpinBox::connect (tf->spinBox_scaleOrb,    SIGNAL(valueChanged(double)),     ctrl, SLOT(orbScaleChanged(double)));
    QSpinBox::connect (tf->spinBox_WTAOrb,      SIGNAL(valueChanged(int)),        ctrl, SLOT(orbWTA_KChanged(int)));
    QSpinBox::connect (tf->spinBox_patchOrb,    SIGNAL(valueChanged(int)),        ctrl, SLOT(orbPatchChanged(int)));
    QCheckBox::connect(tf->checkBox_combineOrb, SIGNAL(clicked(bool)),            ctrl, SLOT(orbCombineChanged(bool)));
    QCheckBox::connect(tf->checkBox_maxProbOrb, SIGNAL(clicked(bool)),            ctrl, SLOT(orbMaxProbChanged(bool)));

    /// SURF
    QCheckBox::connect(tf->surfBox,               SIGNAL(clicked(bool)),          ctrl, SLOT(surfOppChanged(bool)));
    QCheckBox::connect(tf->checkBox_colExtSurf,   SIGNAL(clicked(bool)),          ctrl, SLOT(surfColorExtChanged(bool)));
    QSpinBox::connect (tf->spinBox_octavesSurf,   SIGNAL(valueChanged(int)),      ctrl, SLOT(surfOctavesChanged(int)));
    QSpinBox::connect (tf->spinBox_layersSurf,    SIGNAL(valueChanged(int)),      ctrl, SLOT(surfOctaveLayersChanged(int)));
    QCheckBox::connect(tf->checkBox_extendedSurf, SIGNAL(clicked(bool)),          ctrl, SLOT(surfExtendeChanged(bool)));
    QCheckBox::connect(tf->checkBox_combineSurf,  SIGNAL(clicked(bool)),          ctrl, SLOT(surfCombineChanged(bool)));
    QCheckBox::connect(tf->checkBox_maxProbSurf,  SIGNAL(clicked(bool)),          ctrl, SLOT(surfMaxProbChanged(bool)));

    /// SIFT
    QCheckBox::connect(tf->siftBox,               SIGNAL(clicked(bool)),          ctrl, SLOT(siftOppChanged(bool)));
    QCheckBox::connect(tf->checkBox_colExtSift,   SIGNAL(clicked(bool)),          ctrl, SLOT(siftColorExtChanged(bool)));
    QSpinBox::connect (tf->spinBox_magSift,       SIGNAL(valueChanged(double)),   ctrl, SLOT(siftMagnificationChanged(double)));
    QSpinBox::connect (tf->spinBox_octavesSift,   SIGNAL(valueChanged(int)),      ctrl, SLOT(siftOctavesChanged(int)));
    QCheckBox::connect(tf->checkBox_angSift,      SIGNAL(clicked(bool)),          ctrl, SLOT(siftRecalcAnglesChanged(bool)));
    QCheckBox::connect(tf->checkBox_normSift,     SIGNAL(clicked(bool)),          ctrl, SLOT(siftNormalizeChanged(bool)));
    QCheckBox::connect(tf->checkBox_combineSift,  SIGNAL(clicked(bool)),          ctrl, SLOT(siftCombineChanged(bool)));
    QCheckBox::connect(tf->checkBox_maxProbSift,  SIGNAL(clicked(bool)),          ctrl, SLOT(siftMaxProbChanged(bool)));

    /// BRISK
    QCheckBox::connect(tf->briskBox,               SIGNAL(clicked(bool)),         ctrl, SLOT(briskOppChanged(bool)));
    QCheckBox::connect(tf->checkBox_colExtBrisk,   SIGNAL(clicked(bool)),         ctrl, SLOT(briskColorExtChanged(bool)));
    QSpinBox::connect(tf->spinBox_octavesBrisk,    SIGNAL(valueChanged(int)),     ctrl, SLOT(briskOctavesChanged(int)));
    QSpinBox::connect(tf->spinBox_threshBrisk,     SIGNAL(valueChanged(int)),     ctrl, SLOT(briskThresholdChanged(int)));
    QSpinBox::connect(tf->spinBox_scaleBrisk,      SIGNAL(valueChanged(double)),  ctrl, SLOT(briskScaleChanged(double)));
    QCheckBox::connect(tf->checkBox_combineBrisk,  SIGNAL(clicked(bool)),         ctrl, SLOT(briskCombineChanged(bool)));
    QCheckBox::connect(tf->checkBox_maxProbBrisk,  SIGNAL(clicked(bool)),         ctrl, SLOT(briskMaxProbChanged(bool)));

    /// BRIEF
    QCheckBox::connect(tf->briefBox,                SIGNAL(clicked(bool)),                 ctrl, SLOT(briefOppChanged(bool)));
    QCheckBox::connect(tf->checkBox_colExtBrief,    SIGNAL(clicked(bool)),                 ctrl, SLOT(briskColorExtChanged(bool)));
    QComboBox::connect(tf->comboBox_Brief,          SIGNAL(currentIndexChanged(QString)),  ctrl, SLOT(briefBytesChanged(QString)));

    /// FREAK
    QCheckBox::connect(tf->freakBox,                SIGNAL(clicked(bool)),        ctrl, SLOT(freakOppChanged(bool)));
    QCheckBox::connect(tf->checkBox_colExtFreak,    SIGNAL(clicked(bool)),        ctrl, SLOT(freakColorExtChanged(bool)));
    QSpinBox::connect( tf->spinBox_octavesFreak,    SIGNAL(valueChanged(int)),    ctrl, SLOT(freakOctavesChanged(int)));
    QSpinBox::connect( tf->spinBox_patternFreak,    SIGNAL(valueChanged(double)), ctrl, SLOT(freakPatternScaleChanged(double)));
    QCheckBox::connect(tf->checkBox_oriNormFreak,   SIGNAL(clicked(bool)),        ctrl, SLOT(freakOriNormChanged(bool)));
    QCheckBox::connect(tf->checkBox_scaleNormFreak, SIGNAL(clicked(bool)),        ctrl, SLOT(freakScaleNormChanged(bool)));
    QCheckBox::connect(tf->checkBox_combineFreak,   SIGNAL(clicked(bool)),        ctrl, SLOT(freakCombineChanged(bool)));
    QCheckBox::connect(tf->checkBox_maxProbFreak,   SIGNAL(clicked(bool)),        ctrl, SLOT(freakMaxProbChanged(bool)));

    /// LBP
    QCheckBox::connect(tf->checkBox_colExtlbp,      SIGNAL(clicked(bool)),        ctrl, SLOT(lbpColorExtChanged(bool)));
    /// LTP
    QCheckBox::connect(tf->checkBox_colExtltp,      SIGNAL(clicked(bool)),        ctrl, SLOT(ltpColorExtChanged(bool)));
    QCheckBox::connect(tf->checkBox_combineLTP,     SIGNAL(clicked(bool)),        ctrl, SLOT(ltpCombineChanged(bool)));
    QSpinBox::connect(tf->spinBox_kltp,             SIGNAL(valueChanged(double)), ctrl, SLOT(ltpKChanged(double)));

    /// KEYPOINT
    QSpinBox::connect(tf->spinBox_sizeKeypoint,     SIGNAL(valueChanged(double)), ctrl, SLOT(keypointSizeChanged(double)));
    QSpinBox::connect(tf->spinBox_angleKeypoint,    SIGNAL(valueChanged(double)), ctrl, SLOT(keypointAngleChanged(double)));
    QCheckBox::connect(tf->checkBox_softCrop,       SIGNAL(clicked(bool)),        ctrl, SLOT(keypointCropChanged(bool)));
    QSpinBox::connect(tf->spinBox_useOctaveKeypoint,SIGNAL(valueChanged(int)),    ctrl, SLOT(keypointOctavesChanged(int)));
    QCheckBox::connect(tf->checkBox_calcAngleKeypoint, SIGNAL(clicked(bool)),     ctrl, SLOT(keypointCalcAngleChanged(bool)));
    /// FOREST
    QSpinBox::connect(tf->spinBox_treeMaxDepth,     SIGNAL(valueChanged(int)),    ctrl, SLOT(forest_depthChanged(int)));
    QSpinBox::connect(tf->spinBox_treeMinSampels,   SIGNAL(valueChanged(int)),    ctrl, SLOT(forest_samplesChanged(int)));
    QSpinBox::connect(tf->spinBox_treeRegression,   SIGNAL(valueChanged(double)), ctrl, SLOT(forest_regressionChanged(double)));
    QCheckBox::connect(tf->checkBox_treeSurrogates, SIGNAL(clicked(bool)),        ctrl, SLOT(forest_surrogatesChanged(bool)));
    QSpinBox::connect(tf->spinBox_treeCategories,   SIGNAL(valueChanged(int)),    ctrl, SLOT(forest_categoriesChanged(int)));
    QCheckBox::connect(tf->checkBox_treeVariableImportance, SIGNAL(clicked(bool)),ctrl, SLOT(forest_importanceChanged(bool)));
    QSpinBox::connect(tf->spinBox_treeNactiveVariables, SIGNAL(valueChanged(int)),ctrl, SLOT(forest_nactivesChanged(int)));
    QSpinBox::connect(tf->spinBox_treeMaxTrees,     SIGNAL(valueChanged(int)),    ctrl, SLOT(forest_maxTreesChanged(int)));
    QSpinBox::connect(tf->spinBox_treeAccuracy,     SIGNAL(valueChanged(double)), ctrl, SLOT(forest_accuracyChanged(double)));

    /// FEEDBACK
    QSpinBox::connect(tf->spinBox_gridCellSize,     SIGNAL(valueChanged(int)),    ctrl, SLOT(feedback_gridCellChanged(int)));
    QSpinBox::connect(tf->spinBox_quadCellSize,     SIGNAL(valueChanged(int)),    ctrl, SLOT(feedback_quadMinSizeChanged(int)));
    QSpinBox::connect(tf->spinBox_quadProbTh,       SIGNAL(valueChanged(double)), ctrl, SLOT(feedback_quadMinProbChanged(double)));

    /// PRESET
    QObject::connect(tp.get(), SIGNAL(setForestParams()),                         ctrl,     SLOT(applyForestParams()));
    QObject::connect(tp.get(), SIGNAL(setExtrParams(QString)),                    ctrl,     SLOT(applyExtratorParams(QString)));
    QObject::connect(tp.get(), SIGNAL(setGridParams()),                           ctrl,     SLOT(applyGridParams()));
    QObject::connect(tp.get(), SIGNAL(setQuadParams()),                           ctrl,     SLOT(applyQuadParams()));
    QObject::connect(ctrl,     SIGNAL(paramsForeApplied()),                       tp.get(), SLOT(paramsForeApplied()));
    QObject::connect(ctrl,     SIGNAL(paramsExtrApplied()),                       tp.get(), SLOT(paramsExtrApplied()));
    QObject::connect(ctrl,     SIGNAL(paramsGridApplied()),                       tp.get(), SLOT(paramsGridApplied()));
    QObject::connect(ctrl,     SIGNAL(paramsQuadApplied()),                       tp.get(), SLOT(paramsQuadApplied()));

    Controller::IDPtr entry(Controller::Preferences, Controller::Ptr(ctrl));
    mainWindow->controllers_.insert(entry);
}
