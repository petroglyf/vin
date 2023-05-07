/********************************************************************************
** Form generated from reading UI file 'putitup.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PUTITUP_H
#define UI_PUTITUP_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PutItUp
{
public:
    QAction *actionNew;
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionSave_As;
    QAction *actionQuit;
    QAction *actionGraph_Visualizer;
    QWidget *frame_layout;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QTreeWidget *current_dag;
    QListWidget *available_libs;
    QVBoxLayout *right_vert;
    QScrollArea *options_pane;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *__button_horiz;
    QPushButton *defaults_button;
    QPushButton *create_button;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuTools;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *PutItUp)
    {
        if (PutItUp->objectName().isEmpty())
            PutItUp->setObjectName("PutItUp");
        PutItUp->resize(800, 600);
        actionNew = new QAction(PutItUp);
        actionNew->setObjectName("actionNew");
        actionOpen = new QAction(PutItUp);
        actionOpen->setObjectName("actionOpen");
        actionSave = new QAction(PutItUp);
        actionSave->setObjectName("actionSave");
        actionSave_As = new QAction(PutItUp);
        actionSave_As->setObjectName("actionSave_As");
        actionQuit = new QAction(PutItUp);
        actionQuit->setObjectName("actionQuit");
        actionGraph_Visualizer = new QAction(PutItUp);
        actionGraph_Visualizer->setObjectName("actionGraph_Visualizer");
        frame_layout = new QWidget(PutItUp);
        frame_layout->setObjectName("frame_layout");
        horizontalLayout = new QHBoxLayout(frame_layout);
        horizontalLayout->setObjectName("horizontalLayout");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setSizeConstraint(QLayout::SetMaximumSize);
        current_dag = new QTreeWidget(frame_layout);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        current_dag->setHeaderItem(__qtreewidgetitem);
        current_dag->setObjectName("current_dag");

        verticalLayout->addWidget(current_dag);

        available_libs = new QListWidget(frame_layout);
        available_libs->setObjectName("available_libs");

        verticalLayout->addWidget(available_libs);


        horizontalLayout->addLayout(verticalLayout);

        right_vert = new QVBoxLayout();
        right_vert->setObjectName("right_vert");
        right_vert->setSizeConstraint(QLayout::SetMaximumSize);
        options_pane = new QScrollArea(frame_layout);
        options_pane->setObjectName("options_pane");
        options_pane->setEnabled(false);
        options_pane->setAcceptDrops(false);
        options_pane->setWidgetResizable(true);
        options_pane->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName("scrollAreaWidgetContents");
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 379, 479));
        verticalLayout_2 = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayout_2->setObjectName("verticalLayout_2");
        options_pane->setWidget(scrollAreaWidgetContents);

        right_vert->addWidget(options_pane);

        __button_horiz = new QHBoxLayout();
        __button_horiz->setObjectName("__button_horiz");
        __button_horiz->setSizeConstraint(QLayout::SetDefaultConstraint);
        defaults_button = new QPushButton(frame_layout);
        defaults_button->setObjectName("defaults_button");

        __button_horiz->addWidget(defaults_button);

        create_button = new QPushButton(frame_layout);
        create_button->setObjectName("create_button");

        __button_horiz->addWidget(create_button);


        right_vert->addLayout(__button_horiz);


        horizontalLayout->addLayout(right_vert);

        PutItUp->setCentralWidget(frame_layout);
        menubar = new QMenuBar(PutItUp);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 24));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName("menuFile");
        menuTools = new QMenu(menubar);
        menuTools->setObjectName("menuTools");
        PutItUp->setMenuBar(menubar);
        statusbar = new QStatusBar(PutItUp);
        statusbar->setObjectName("statusbar");
        PutItUp->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuTools->menuAction());
        menuFile->addAction(actionNew);
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionSave);
        menuFile->addAction(actionSave_As);
        menuFile->addAction(actionQuit);
        menuTools->addAction(actionGraph_Visualizer);

        retranslateUi(PutItUp);

        QMetaObject::connectSlotsByName(PutItUp);
    } // setupUi

    void retranslateUi(QMainWindow *PutItUp)
    {
        PutItUp->setWindowTitle(QCoreApplication::translate("PutItUp", "Oculator visualizer", nullptr));
        actionNew->setText(QCoreApplication::translate("PutItUp", "New", nullptr));
#if QT_CONFIG(shortcut)
        actionNew->setShortcut(QCoreApplication::translate("PutItUp", "Ctrl+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actionOpen->setText(QCoreApplication::translate("PutItUp", "Open", nullptr));
#if QT_CONFIG(shortcut)
        actionOpen->setShortcut(QCoreApplication::translate("PutItUp", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSave->setText(QCoreApplication::translate("PutItUp", "Save", nullptr));
#if QT_CONFIG(shortcut)
        actionSave->setShortcut(QCoreApplication::translate("PutItUp", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSave_As->setText(QCoreApplication::translate("PutItUp", "Save As..", nullptr));
#if QT_CONFIG(shortcut)
        actionSave_As->setShortcut(QCoreApplication::translate("PutItUp", "Ctrl+A", nullptr));
#endif // QT_CONFIG(shortcut)
        actionQuit->setText(QCoreApplication::translate("PutItUp", "Quit", nullptr));
#if QT_CONFIG(shortcut)
        actionQuit->setShortcut(QCoreApplication::translate("PutItUp", "Ctrl+Q", nullptr));
#endif // QT_CONFIG(shortcut)
        actionGraph_Visualizer->setText(QCoreApplication::translate("PutItUp", "Graph Visualizer", nullptr));
#if QT_CONFIG(shortcut)
        actionGraph_Visualizer->setShortcut(QCoreApplication::translate("PutItUp", "Ctrl+T", nullptr));
#endif // QT_CONFIG(shortcut)
        defaults_button->setText(QCoreApplication::translate("PutItUp", "Accept Defaults", nullptr));
        create_button->setText(QCoreApplication::translate("PutItUp", "Create", nullptr));
        menuFile->setTitle(QCoreApplication::translate("PutItUp", "File", nullptr));
        menuTools->setTitle(QCoreApplication::translate("PutItUp", "Tools", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PutItUp: public Ui_PutItUp {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PUTITUP_H
