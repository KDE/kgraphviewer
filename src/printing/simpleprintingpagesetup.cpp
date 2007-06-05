/* This file is part of KGraphViewer.
   Copyright (C) 2005-2006 Gaël de Chalendar <kleag@free.fr>

   KGraphViewer is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* This file was part of the KDE project
   Copyright (C) 2005 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 */

#include "simpleprintingpagesetup.h"
#include "simpleprintingpagesetupbase.h"
#include "simpleprintingsettings.h"
#include "simpleprintingcommand.h"
#include "simpleprintpreviewwindow.h"
#include "dotgraphview.h"

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kfontdialog.h>
#include <kurllabel.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kprinter.h>
#include <kpushbutton.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kurl.h>

#include <qlabel.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qslider.h>
#include <qradiobutton.h>

#include <iostream>


KGVSimplePrintingPageSetup::KGVSimplePrintingPageSetup(
    KGVSimplePrintingCommand* command, DotGraphView *mainWin, QWidget *parent, 
	  QMap<QString,QString>* args )
	: QWidget( parent, "KGVSimplePrintingPageSetup" )
	, m_settings( command->engine()->settings() )
  , m_command(command)
{
	// object to print
	bool ok = args;
  m_graphView = mainWin;
	ok = m_graphView;

	bool printPreview = false;
	bool print = false;
	bool pageSetup = false;
	if (ok) {
		printPreview = (*args)["action"]=="printPreview";
		print = (*args)["action"]=="print";
		pageSetup = (*args)["action"]=="pageSetup";
		ok = printPreview || print || pageSetup;
	}
  

	// settings
//! @todo default?
	m_unit = KLocale::Metric == KGlobal::locale()->measureSystem() ? KgvUnit::U_CM : KgvUnit::U_INCH;

	// GUI
	QVBoxLayout *lyr = new QVBoxLayout(this);
	m_contents = new KGVSimplePrintingPageSetupBase(this, "KGVSimplePrintingPageSetupBase");
	lyr->addWidget(m_contents);

	setFocusPolicy(WheelFocus);
	m_contents->setFocusProxy(m_contents->headerTitleLineEdit);

	m_contents->printButton->setIconSet( KStdGuiItem::print().iconSet(KIcon::Small) );
	m_contents->printButton->setText( KStdGuiItem::print().text() );
	connect(m_contents->printButton, SIGNAL(clicked()), this, SLOT(slotPrint()));

	m_contents->printPreviewButton->setIconSet( SmallIconSet("filequickprint") );
	m_contents->printPreviewButton->setText( i18n("Print Previe&w...") );
	connect(m_contents->printPreviewButton, SIGNAL(clicked()), this, SLOT(slotPrintPreview()));

	m_contents->iconLabel->setFixedWidth(32+6);
	m_contents->iconLabel->setPixmap( DesktopIcon("document", 32) );
	QWhatsThis::add(m_contents->headerTitleFontButton, i18n("Changes font for title text."));
	connect(m_contents->headerTitleFontButton, SIGNAL(clicked()), 
		this, SLOT(slotChangeTitleFont()));

	if (m_graphView) {
		m_origCaptionLabelText = m_contents->captionLabel->text();
    m_contents->headerTitleLineEdit->setText((*args)["title"]);
		m_contents->captionLabel->setText( m_origCaptionLabelText.arg( KURL::fromPathOrURL(m_graphView->dotFileName()).fileName() ) );
	}
	connect(m_contents->headerTitleLineEdit,SIGNAL(textChanged(const QString&)), 
		this, SLOT(slotTitleTextChanged(const QString&)));
	m_contents->headerTitleLineEdit->setFont( m_settings->pageTitleFont );

	QWhatsThis::add(m_contents->saveSetupLink, i18n("Saves settings for this setup as default."));
	connect(m_contents->saveSetupLink, SIGNAL(leftClickedURL()), this, SLOT(slotSaveSetup()));

	QWhatsThis::add(m_contents->addDateTimeCheckbox, i18n("Adds date and time to the header."));
	QWhatsThis::add(m_contents->addPageNumbersCheckbox, i18n("Adds page numbers to the footer."));
	QWhatsThis::add(m_contents->addTableBordersCheckbox, i18n("Adds table borders."));
	
#ifdef KGV_NO_UNFINISHED 
	m_contents->addDateTimeCheckbox->hide();
	m_contents->addPageNumbersCheckbox->hide();
#endif

	updatePageLayoutAndUnitInfo();
	QWhatsThis::add(m_contents->changePageSizeAndMarginsButton, 
		i18n("Changes page size and margins."));
	connect(m_contents->changePageSizeAndMarginsButton, SIGNAL(clicked()), 
		this, SLOT(slotChangePageSizeAndMargins()));

	connect(m_contents->addPageNumbersCheckbox, SIGNAL(toggled(bool)), 
		this, SLOT(slotAddPageNumbersCheckboxToggled(bool)));
	connect(m_contents->addDateTimeCheckbox, SIGNAL(toggled(bool)), 
		this, SLOT(slotAddDateTimeCheckboxToggled(bool)));
  connect(m_contents->addTableBordersCheckbox, SIGNAL(toggled(bool)), 
    this, SLOT(slotAddTableBordersCheckboxToggled(bool)));


// 
	m_contents->addPageNumbersCheckbox->setChecked( m_settings->addPageNumbers );
	m_contents->addDateTimeCheckbox->setChecked( m_settings->addDateAndTime );
  m_contents->addTableBordersCheckbox->setChecked( m_settings->addTableBorders );
	setDirty(false);

	// 	clear it back to false after widgets initialization
	m_printPreviewNeedsReloading = false;

	if (printPreview)
		QTimer::singleShot(50, this, SLOT(printPreview()));
	else if (print)
		QTimer::singleShot(50, this, SLOT(print()));
	connect(this, SIGNAL(print()), m_graphView, SLOT(print()));
	connect(this, SIGNAL(printPreview()), m_graphView, SLOT(printPreview()));

  connect(m_contents->closeButton, SIGNAL(clicked()), this, SLOT(slotClose()));

  
  m_contents->horizFitNumInput->setRange(1,m_command->engine()->maxHorizFit(),1,true);
  m_contents->vertFitNumInput->setRange(1,m_command->engine()->maxVertFit(),1,true);
  m_settings->horizFitting = m_command->engine()->maxHorizFit();
  m_settings->vertFitting = m_command->engine()->maxVertFit();
  m_contents->horizFitNumInput->setValue(m_settings->horizFitting);
  m_contents->vertFitNumInput->setValue(m_settings->vertFitting);
  connect(m_contents->maintainAspectButton, SIGNAL(clicked()), 
    this, SLOT(slotMaintainAspectButtonToggled()));
  connect(m_contents->horizFitNumInput, SIGNAL(valueChanged(int)),
    this, SLOT(slotHorizFitChanged(int)));
  connect(m_contents->vertFitNumInput, SIGNAL(valueChanged(int)),
    this, SLOT(slotVertFitChanged(int)));

  m_fittingModeButtons.insert((QButton*)(m_contents->naturalSizeRadioButton), NaturalSize);
  m_fittingModeButtons.insert((QButton*)(m_contents->fitToOnePageRadioButton), FitToOnePage);
  m_fittingModeButtons.insert((QButton*)(m_contents->fitToSeveralPagesRadioButton), FitToSeveralPages);
  connect(&m_fittingModeButtons, SIGNAL(clicked(int)), 
    this, SLOT(slotFittingButtonClicked(int)));

  m_fittingModeButtons.setButton(m_settings->fittingMode);
  if (m_settings->fittingMode != FitToSeveralPages)
  {
    m_contents->horizFitNumInput->setEnabled(false);
    m_contents->vertFitNumInput->setEnabled(false);
    m_contents->maintainAspectButton->setEnabled(false);
  }

  QString chainStatePixString = KGlobal::dirs()-> findResource("appdata", "pics/chain.png");
  if (!m_settings->chainedFittings)
  {
    chainStatePixString = KGlobal::dirs()-> findResource("appdata", "pics/chain-broken.png");
  }
  if (chainStatePixString.isNull())
  {
    std::cerr << "chain state pixmap not found !" << std::endl;
  }
  m_contents->maintainAspectButton->setPixmap(QPixmap(chainStatePixString));

  // hides currently unused title label
  m_contents->headerTitleLineEdit->setText(i18n("Chosen font looks like this"));
  m_contents->headerTitleLineEdit->setReadOnly(true);
}

KGVSimplePrintingPageSetup::~KGVSimplePrintingPageSetup()
{
}

void KGVSimplePrintingPageSetup::updatePageLayoutAndUnitInfo()
{
  QString s;
  if (m_settings->pageLayout.format == PG_CUSTOM) {
    s += QString(" (%1 %2 x %3 %4)")
      .arg(m_settings->pageLayout.ptWidth).arg(KgvUnit::unitName(m_unit))
      .arg(m_settings->pageLayout.ptHeight).arg(KgvUnit::unitName(m_unit));
  }
  else
    s += KgvPageFormat::name(m_settings->pageLayout.format);
  s += QString(", ")
   + (m_settings->pageLayout.orientation == PG_PORTRAIT ? i18n("Portrait") : i18n("Landscape"))
   + ", " + i18n("margins:")
   + ' ' + KgvUnit::toUserStringValue(m_settings->pageLayout.ptLeft, m_unit)
   + '/' + KgvUnit::toUserStringValue(m_settings->pageLayout.ptRight, m_unit)
   + '/' + KgvUnit::toUserStringValue(m_settings->pageLayout.ptTop, m_unit)
   + '/' + KgvUnit::toUserStringValue(m_settings->pageLayout.ptBottom, m_unit)
   + ' ' + KgvUnit::unitName(m_unit);
  m_contents->pageSizeAndMarginsLabel->setText( s );
  m_contents->horizFitNumInput->setRange(1,m_command->engine()->maxHorizFit(),1,true);
  m_contents->vertFitNumInput->setRange(1,m_command->engine()->maxVertFit(),1,true);
}

void KGVSimplePrintingPageSetup::setDirty(bool set)
{
  m_contents->saveSetupLink->setEnabled(set);
  if (set)
  {
    m_printPreviewNeedsReloading = true;
    m_command->engine()->clear();
    emit needsRedraw();
  }
}

void KGVSimplePrintingPageSetup::slotSaveSetup()
{
	m_settings->save();
	setDirty(false);
}

void KGVSimplePrintingPageSetup::slotPrint()
{
	emit print();
}

void KGVSimplePrintingPageSetup::slotPrintPreview()
{
	emit printPreview();
	m_printPreviewNeedsReloading = false;
}

void KGVSimplePrintingPageSetup::slotTitleTextChanged(const QString&)
{
	if (m_contents->headerTitleLineEdit->isModified()) {
		m_printPreviewNeedsReloading = true;
	}
		
	m_contents->headerTitleLineEdit->clearModified();
  setDirty(true);
}

void KGVSimplePrintingPageSetup::slotChangeTitleFont()
{
	if (QDialog::Accepted != KFontDialog::getFont(m_settings->pageTitleFont, false, this))
		return;
	m_contents->headerTitleLineEdit->setFont( m_settings->pageTitleFont );
	setDirty(true);
}

void KGVSimplePrintingPageSetup::slotChangePageSizeAndMargins()
{
	KgvHeadFoot headfoot; //dummy
//   std::cerr << "PageLayout before: " << m_settings->pageLayout.orientation << std::endl;
	if (int(QDialog::Accepted) != KgvPageLayoutDia::pageLayout( 
		m_settings->pageLayout, headfoot, FORMAT_AND_BORDERS | DISABLE_UNIT, m_unit, this ))
		return;

//   std::cerr << "PageLayout after: " << m_settings->pageLayout.orientation << std::endl;
	//update
	updatePageLayoutAndUnitInfo();
	setDirty(true);
}

void KGVSimplePrintingPageSetup::slotAddPageNumbersCheckboxToggled(bool set)
{
	m_settings->addPageNumbers = set;
	setDirty(true);
}

void KGVSimplePrintingPageSetup::slotAddDateTimeCheckboxToggled(bool set)
{
	m_settings->addDateAndTime = set;
	setDirty(true);
}

void KGVSimplePrintingPageSetup::slotAddTableBordersCheckboxToggled(bool set)
{
	m_settings->addTableBorders = set;
	setDirty(true);
}

void KGVSimplePrintingPageSetup::slotFittingButtonClicked(int id)
{
  if (id == NaturalSize)
  {
    m_settings->fitToOnePage = false;
    m_contents->horizFitNumInput->setEnabled(false);
    m_contents->vertFitNumInput->setEnabled(false);
    m_contents->maintainAspectButton->setEnabled(false);
  }
  else if (id == FitToOnePage)
  {
    m_settings->fitToOnePage = true;
    m_contents->horizFitNumInput->setEnabled(false);
    m_contents->vertFitNumInput->setEnabled(false);
    m_contents->maintainAspectButton->setEnabled(false);
  }
  else if (id == FitToSeveralPages)
  {
    m_settings->fitToOnePage = false;
    m_contents->horizFitNumInput->setEnabled(true);
    m_contents->vertFitNumInput->setEnabled(true);
    m_contents->maintainAspectButton->setEnabled(true);
  }
  setDirty(true);
}

void KGVSimplePrintingPageSetup::slotMaintainAspectButtonToggled()
{
  if (m_settings->chainedFittings)
  {
    QString chainBreakPixString = KGlobal::dirs()-> findResource("appdata", "pics/chain-broken.png");
    if (chainBreakPixString.isNull())
    {
      std::cerr << "chain break pixmap not found !" << std::endl;
    }
    m_contents->maintainAspectButton->setPixmap(QPixmap(chainBreakPixString));
    m_settings->chainedFittings = false;
  }
  else
  {
    QString chainPixString = KGlobal::dirs()-> findResource("appdata", "pics/chain.png");
    if (chainPixString.isNull())
    {
      std::cerr << "chain pixmap not found !" << std::endl;
    }
    m_contents->maintainAspectButton->setPixmap(QPixmap(chainPixString));
    m_settings->chainedFittings = true;
  }
  emit needsRedraw();
}

void KGVSimplePrintingPageSetup::slotClose()
{
  parentWidget()->hide();
}

void KGVSimplePrintingPageSetup::slotHorizFitChanged(int newValue)
{
  m_settings->horizFitting = newValue;
  m_printPreviewNeedsReloading = true;
  emit needsRedraw();
}

void KGVSimplePrintingPageSetup::slotVertFitChanged(int newValue)
{
  m_settings->vertFitting = newValue;
  m_printPreviewNeedsReloading = true;
  emit needsRedraw();
}

#include "simpleprintingpagesetup.moc"
