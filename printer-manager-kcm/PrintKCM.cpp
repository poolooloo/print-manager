/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "PrintKCM.h"

#include "PrinterModel.h"
#include "PrinterDelegate.h"
#include "PrinterDescription.h"
#include "SystemPreferences.h"
#include "ConfigureDialog.h"

#include <KMessageBox>
#include <KGenericFactory>
#include <KAboutData>
#include <KTitleWidget>
#include <KIcon>

#include <QTimer>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QVBoxLayout>
#include <QCups.h>
#include <cups/cups.h>

K_PLUGIN_FACTORY(PrintKCMFactory, registerPlugin<PrintKCM>();)
K_EXPORT_PLUGIN(PrintKCMFactory("kcm_print"))

PrintKCM::PrintKCM(QWidget *parent, const QVariantList &args)
    : KCModule(PrintKCMFactory::componentData(), parent, args),
      m_hasError(true)
{
    KAboutData *aboutData;
    aboutData = new KAboutData("kcm_print",
                               "kcm_print",
                               ki18n("Print settings"),
                               "0.1",
                               ki18n("Print settings"),
                               KAboutData::License_GPL,
                               ki18n("(C) 2010 Daniel Nicoletti"));
    setAboutData(aboutData);
    setButtons(Help);

    setupUi(this);

    addPB->setIcon(KIcon("list-add"));
    removePB->setIcon(KIcon("list-remove"));
    preferencesPB->setIcon(KIcon("configure"));
    configurePrinterPB->setIcon(KIcon("configure"));

    m_model = new PrinterModel(winId(), this);
    printersTV->setModel(m_model);
    printersTV->setItemDelegate(new PrinterDelegate(this));
    connect(printersTV->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(update()));
    connect(printersTV->model(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(update()));
    connect(printersTV->model(), SIGNAL(error(bool, const QString &, const QString &)),
            this, SLOT(error(bool, const QString &, const QString &)));

    // Create the PrinterDescription before we try to select a printer
    m_printerDesc = new PrinterDescription(scrollAreaWidgetContents);
    m_printerDesc->hide();

    // widget for when we don't have a printer
    m_noPrinter = new QWidget(scrollAreaWidgetContents);
    KTitleWidget *widget = new KTitleWidget(m_noPrinter);
    widget->setText(i18n("You have no printers"),
                         KTitleWidget::InfoMessage);
    widget->setComment(i18n("If you want to add one just click on the plus sign below the list"));

    QVBoxLayout *vertLayout = new QVBoxLayout(m_noPrinter);
    vertLayout->addStretch();
    vertLayout->addWidget(widget);
    vertLayout->addStretch();

    // if we get an error from the server we use this widget
    m_serverError = new QWidget(scrollAreaWidgetContents);
    m_serverErrorW = new KTitleWidget(m_serverError);
    vertLayout = new QVBoxLayout(m_serverError);
    vertLayout->addStretch();
    vertLayout->addWidget(m_serverErrorW);
    vertLayout->addStretch();

    // the stacked layout allow us to chose which widget to show
    m_stackedLayout = new QStackedLayout(scrollAreaWidgetContents);
    m_stackedLayout->addWidget(m_serverError);
    m_stackedLayout->addWidget(m_noPrinter);
    m_stackedLayout->addWidget(m_printerDesc);
    scrollAreaWidgetContents->setLayout(m_stackedLayout);

    // Force the model update AFTER we setup the error signal
    m_model->update();

    // select the first printer if there are printers
    if (m_model->rowCount()) {
        printersTV->selectionModel()->select(m_model->index(0, 0), QItemSelectionModel::Select);
    }
}

void PrintKCM::error(bool hasError, const QString &errorTitle, const QString &errorMsg)
{
    if (hasError) {
        m_serverErrorW->setText(errorTitle, KTitleWidget::ErrorMessage);
        m_serverErrorW->setComment(errorMsg);
        if (m_stackedLayout->widget() != m_serverError) {
            m_stackedLayout->setCurrentWidget(m_serverError);
        }
    }

    if (m_hasError != hasError) {
        addPB->setEnabled(!hasError);
        removePB->setEnabled(false);
        configurePrinterPB->setEnabled(false);
        preferencesPB->setEnabled(!hasError);
        printersTV->setEnabled(!hasError);
        m_hasError = hasError;
        // Force an update
        update();
    }
}

PrintKCM::~PrintKCM()
{
}

void PrintKCM::update()
{
    if (m_model->rowCount()) {
        if (m_stackedLayout->widget() != m_printerDesc) {
            m_stackedLayout->setCurrentWidget(m_printerDesc);
        }

        QItemSelection selection;
        // we need to map the selection to source to get the real indexes
        selection = printersTV->selectionModel()->selection();
        // select the first printer if there are printers
        if (selection.indexes().isEmpty()) {
            printersTV->selectionModel()->select(m_model->index(0, 0), QItemSelectionModel::Select);
            return;
        }

        QModelIndex index = selection.indexes().first();
        QString destName = index.data(PrinterModel::DestName).toString();
        if (m_printerDesc->destName() != destName) {
            m_printerDesc->setPrinterIcon(index.data(Qt::DecorationRole).value<QIcon>());
            int type = index.data(PrinterModel::DestType).toInt();
            // If we remove discovered printers, they will come
            // back to hunt us a bit later
            removePB->setEnabled(!(type & CUPS_PRINTER_DISCOVERED));
            configurePrinterPB->setEnabled(true);
        }
        m_printerDesc->setDestName(index.data(PrinterModel::DestName).toString(),
                                   index.data(PrinterModel::DestDescription).toString(),
                                   index.data(PrinterModel::DestIsClass).toBool());
        m_printerDesc->setLocation(index.data(PrinterModel::DestLocation).toString());
        m_printerDesc->setStatus(index.data(PrinterModel::DestStatus).toString());
        m_printerDesc->setKind(index.data(PrinterModel::DestKind).toString());
        m_printerDesc->setIsShared(index.data(PrinterModel::DestIsShared).toBool());
        m_printerDesc->setIsDefault(index.data(PrinterModel::DestIsDefault).toBool());
        m_printerDesc->setCommands(index.data(PrinterModel::DestCommands).toStringList());
        if (m_printerDesc->needMarkerLevels(index.data(PrinterModel::DestMarkerChangeTime).toInt())) {
            m_printerDesc->setMarkerLevels(index.data(PrinterModel::DestMarkerLevels));
            m_printerDesc->setMarkerColors(index.data(PrinterModel::DestMarkerColors));
            m_printerDesc->setMarkerNames(index.data(PrinterModel::DestMarkerNames));
            m_printerDesc->setMarkerTypes(index.data(PrinterModel::DestMarkerTypes));
        }
    } else if (m_stackedLayout->widget() != m_noPrinter) {
        // the model is empty and no problem happened
        m_stackedLayout->setCurrentWidget(m_noPrinter);
        // disable the printer action buttons if there is nothing to selected
        removePB->setEnabled(false);
        configurePrinterPB->setEnabled(false);
    }
}

void PrintKCM::on_addPB_clicked()
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall("org.kde.AddPrinter",
                                             "/",
                                             "org.kde.AddPrinter",
                                             QLatin1String("AddPrinter"));
    // Use our own cached tid to avoid crashes
    message << qVariantFromValue(QString());
    QDBusConnection::sessionBus().call(message);
}

void PrintKCM::on_removePB_clicked()
{
    QItemSelection selection;
    // we need to map the selection to source to get the real indexes
    selection = printersTV->selectionModel()->selection();
    // enable or disable the job action buttons if something is selected
    if (!selection.indexes().isEmpty()) {
        QModelIndex index = selection.indexes().first();
        int resp;
        QString msg, title;
        if (index.data(PrinterModel::DestIsClass).toBool()) {
            title = i18n("Remove class");
            msg = i18n("Are you sure you want to remove the class '%1'?",
                       index.data(Qt::DisplayRole).toString());
        } else {
            title = i18n("Remove printer");
            msg = i18n("Are you sure you want to remove the printer '%1'?",
                       index.data(Qt::DisplayRole).toString());
        }
        resp = KMessageBox::questionYesNo(this, msg, title);
        if (resp == KMessageBox::Yes) {
            QCups::Result *ret = QCups::deletePrinter(index.data(PrinterModel::DestName).toString());
            ret->waitTillFinished();
            ret->deleteLater();
        }
    }
}

void PrintKCM::on_configurePrinterPB_clicked()
{
    QItemSelection selection;
    // we need to map the selection to source to get the real indexes
    selection = printersTV->selectionModel()->selection();
    // enable or disable the job action buttons if something is selected
    if (!selection.indexes().isEmpty()) {
        QModelIndex index = selection.indexes().at(0);
        QCups::ConfigureDialog *dlg;
        dlg= new QCups::ConfigureDialog(index.data(PrinterModel::DestName).toString(),
                                        index.data(PrinterModel::DestIsClass).toBool(),
                                        this);
        dlg->show();
    }
}

void PrintKCM::on_preferencesPB_clicked()
{
    SystemPreferences *dlg = new SystemPreferences(this);
    dlg->show();
}
