#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QLabel>
#include <QTimer>

#include "editor_core/sq_viewport.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Create the dock manager. Because the parent parameter is a QMainWindow
    // the dock manager registers itself as the central widget.
	m_DockManager = new ads::CDockManager(this);

    const char* provider_arguments[] = { "provider", "--d3d12"};
    auto provider1 = 
		skr_run_process("cross-process.exe", provider_arguments, 2, "provider-d3d.log");
    auto provider_id1 = skr_get_process_id(provider1);

    const char* provider_arguments2[] = { "provider", "--vulkan"};
    auto provider2 = 
		skr_run_process("cross-process.exe", provider_arguments2, 2, "provider-vk.log");
    auto provider_id2 = skr_get_process_id(provider2);

	skq::SQViewport* vport_d3d = new skq::SQViewport(nullptr, provider_id1);
	skq::SQViewport* vport_vk = new skq::SQViewport(nullptr, provider_id2);

	// Create a dock widget with the title Label 1 and set the created label
	// as the dock widget content
	ads::CDockWidget* DockWidget = new ads::CDockWidget("Viewport D3D");
	DockWidget->setWidget(vport_d3d);

	ads::CDockWidget* DockWidget2 = new ads::CDockWidget("Viewport Vulkan");
	DockWidget2->setWidget(vport_vk);

	// Add the toggleViewAction of the dock widget to the menu to give
	// the user the possibility to show the dock widget if it has been closed
	ui->menuView->addAction(DockWidget->toggleViewAction());
	ui->menuView->addAction(DockWidget2->toggleViewAction());

	// Add the dock widget to the top dock widget area
	m_DockManager->addDockWidget(ads::TopDockWidgetArea, DockWidget);
	m_DockManager->addDockWidget(ads::TopDockWidgetArea, DockWidget2);
}

MainWindow::~MainWindow()
{
    delete ui;
}

