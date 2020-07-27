#include "combomenuwidget.h"

#include <QPainter>
#include <QFocusEvent>
#include <QFont>
#include "commongraphics/commongraphics.h"
#include "utils/makecustomshadow.h"
#include "graphicresources/fontmanager.h"
#include "dpiscalemanager.h"

namespace CommonWidgets {

ComboMenuWidget::ComboMenuWidget(QWidget *parent) : QWidget(parent)
  , roundedCorner_(7)
  , menuListPosY_(0)
{

#ifdef Q_OS_WIN
    // Qt::SubWindow will make windowmanager icon change color -- but it won't interfere with screen resizing properly during a scaling change (Qt::PopUp -- probably qt bug)
    // Qt::SubWindow will also take focus in/out events properly (unlike Qt::Tool and Qt::ToolTip)
    // Until Popup bug is fixed Qt::SubWindow is best option
    setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
#else
    // Menu not visible on Mac with when it is a Qt::SubWindow
    // Popup will prevent mainwindow WindowDeactivate event from hiding the app while using the combobox
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
#endif
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFocusPolicy(Qt::StrongFocus);

    //font_ = *FontManager::instance().getFont(12*g_pixelRatio, false);

    layout_ = new QVBoxLayout();
    layout_->setMargin(0);
    layout_->setSpacing(0);
    layout_->setSizeConstraint(QLayout::SetFixedSize);

    menuListWidget_ = new QWidget(this);
    menuListWidget_->setLayout(layout_);
    menuListWidget_->show();

    scrollBar_ = new VerticalScrollBarWidget(5, 90, this);
    connect(scrollBar_, SIGNAL(moved(double)), SLOT(onScrollBarMoved(double)));
    scrollBar_->setBackgroundColor(Qt::white);
    scrollBar_->setForegroundColor(Qt::black);
    scrollBar_->show();

    QString layoutStyleSheet = " border: none; "
                               " background-color: rgba(0,0,0, 0%) ";
    menuListWidget_->setStyleSheet(layoutStyleSheet);
}

void ComboMenuWidget::addItem(QString text, const QVariant &data)
{
    ComboMenuWidgetButton *button = new ComboMenuWidgetButton(data);
    QFont *font = FontManager::instance().getFont(12, false);
    button->setFont(*font);
    button->setCheckable(true);

    connect(button, SIGNAL(pressed()), SLOT(onButtonClick()));
    connect(button, SIGNAL(hoverEnter()), SLOT(onButtonHoverEnter()));

    items_.append(button);

    QFontMetrics fm(*font);
    button->setWidthUnscaled(fm.boundingRect(button->text()).width() + 30);
    button->setHeightUnscaled(STEP_SIZE);
    button->setText(text);

    layout_->addWidget(button);
    updateScaling();
}

void ComboMenuWidget::removeItem(int index)
{
    ComboMenuWidgetButton *button = items_.at(index);

    items_.removeAt(index);
    layout_->removeWidget(button);

    if (button != nullptr)
    {
        delete button;
        button = nullptr;
    }
}

void ComboMenuWidget::navigateItemToTop(int index)
{
    if (index >= 0 && index < items_.count())
    {
        int newValue = -index*STEP_SIZE*G_SCALE;
        moveListPos(newValue);
    }
}

void ComboMenuWidget::activateItem(int index)
{
    if (index >= 0 && index < items_.count())
    {
        ComboMenuWidgetButton *button = items_[index];
        button->setChecked(true); //  use checked to signify the currently selected button
    }
}

void ComboMenuWidget::clearItems()
{
    for (auto item : items_)
    {
        Q_UNUSED(item);
        removeItem(0);
    }
}

int ComboMenuWidget::visibleItems()
{
    if (items_.length() > MAX_ITEMS_SHOWING)
    {
        return MAX_ITEMS_SHOWING;
    }
    else
    {
        return items_.length();
    }
}

int ComboMenuWidget::itemHeight()
{
    return STEP_SIZE * G_SCALE;
}

int ComboMenuWidget::indexOfLastToppableItem()
{
    return items_.length() - visibleItems();
}

QString ComboMenuWidget::textOfItem(int index)
{
    QString text = "";

    if (index >= 0 && index < items_.length())
    {
        text = items_[index]->text();
    }

    return text;
}

void ComboMenuWidget::onButtonClick()
{
    ComboMenuWidgetButton * button = static_cast<ComboMenuWidgetButton *>(sender());

    if (button != nullptr)
    {
        emit itemClicked(button->text(), button->data());
    }
}

void ComboMenuWidget::onButtonHoverEnter()
{
    // override any currently selected buttons
    for (auto item : items_)
    {
        item->setChecked(false);
    }
}

QPixmap ComboMenuWidget::getCurrentPixmapShape()
{
    QPixmap tempPixmap(geometry().width(), geometry().height());
    tempPixmap.fill(Qt::transparent);

    QPainter painter(&tempPixmap);
    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawRoundRect(roundedBackgroundRect_,
                          roundedCorner_, roundedCorner_);
    return tempPixmap;
}

int ComboMenuWidget::itemCount()
{
    return items_.count();
}

void ComboMenuWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // shadow
    QPixmap shape = getCurrentPixmapShape();
    QPixmap shadow = MakeCustomShadow::makeShadowForPixmap(shape, 0, SHADOW_SIZE, Qt::black);

    QPixmap scaledShadow = shadow.scaled(shape.width(), shape.height());
    painter.drawPixmap(0, 0, scaledShadow);

    // background
    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawRoundRect(roundedBackgroundRect_,
                          roundedCorner_, roundedCorner_);
}

void ComboMenuWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        hide();
    }
    else if (event->key() == Qt::Key_R)
    {
        emit requestScalingUpdate();
    }
}

void ComboMenuWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    emit hidden();
}

void ComboMenuWidget::wheelEvent(QWheelEvent *event)
{
    const int change = 20;
    if (event->delta() > 0) // scroll up
    {
        int newY = menuListPosY_ + change; // moves inner down
        moveListPos(newY);
    }
    else // scroll down
    {
        int newY = menuListPosY_ - change; // moves inner up
        moveListPos(newY);
    }
}

void ComboMenuWidget::focusOutEvent(QFocusEvent *event)
{
    hide();
}

void ComboMenuWidget::onScrollBarMoved(double posPercentY)
{
    // assumes scrollbar will not send strange value
    int posY = posPercentY * menuListWidget_->geometry().height();
    menuListPosY_ = -posY;

    updateScaling();
}

ComboMenuWidgetButton *ComboMenuWidget::itemByName(QString text)
{
    ComboMenuWidgetButton *result = nullptr;

    for (auto item : items_)
    {
        if (text == item->text())
        {
            result = item;
            break;
        }
    }

    return result;
}

void ComboMenuWidget::moveListPos(int newY)
{
    // not too high, not too low
    if ((newY <= 0) &&
        (maxViewportHeight_ - newY <= menuListWidget_->geometry().height()))
    {
        menuListPosY_ = newY;
        updateScaling();
    }
}

int ComboMenuWidget::indexOfItemByName(QString text)
{
    int found = -1;
    for (int i = 0; i < items_.length(); i++)
    {
        if (text == items_[i]->text())
        {
            found = i;
            break;
        }
    }

    return found;
}

int ComboMenuWidget::indexByButton(ComboMenuWidgetButton *button)
{
    int found = -1;
    for (int index = 0; index < items_.length(); index++)
    {
        if (items_[index] == button)
        {
            found = index;
            break;
        }
    }
    return found;
}

int ComboMenuWidget::topMargin()
{
    return (SPACER_SIZE + SHADOW_SIZE) * G_SCALE;
}

int ComboMenuWidget::sideMargin()
{
    return SHADOW_SIZE * G_SCALE;
}

void ComboMenuWidget::updateScaling()
{
    // update button widths
    QFont *font = FontManager::instance().getFont(12, false);
    for (auto buttonItem : items_)
    {
        ComboMenuWidgetButton * button  = static_cast<ComboMenuWidgetButton *>(buttonItem);
        button->setFont(*font);
        QFontMetrics fm(*font);
        button->setWidthUnscaled(fm.boundingRect(button->text()).width() + 30);
    }

    // resize buttons max button width
    int allButtonsHeight = 0;
    int buttonWidth = largestButtonWidth(); // no scaling since font takes care of it
    if (buttonWidth < 55 ) buttonWidth = 55 ; // minimum width

    for (auto buttonItem : items_)
    {
        ComboMenuWidgetButton * button  = static_cast<ComboMenuWidgetButton *>(buttonItem);
        button->setFixedHeight(STEP_SIZE * G_SCALE);
        button->setFixedWidth(buttonWidth);
        allButtonsHeight += STEP_SIZE;
    }

    // resize list widget
    QRect rect(sideMargin(), menuListPosY_ + topMargin(), buttonWidth, allButtonsHeight * G_SCALE);
    menuListWidget_->setGeometry(rect);

    // restrict height of list viewport
    maxViewportHeight_ = MAX_ITEMS_SHOWING * STEP_SIZE * G_SCALE;
    int restrictedHeight = allButtonsHeight * G_SCALE;
    if (restrictedHeight > maxViewportHeight_) restrictedHeight = maxViewportHeight_;

    // clip menu list
    clippingRegion_ = QRect(0, -menuListPosY_,
                            buttonWidth,
                            restrictedHeight);
    menuListWidget_->setMask(clippingRegion_);

    // update rounded background size
    roundedBackgroundRect_ = QRect(SHADOW_SIZE*G_SCALE, SHADOW_SIZE*G_SCALE,
                                   buttonWidth,
                                   restrictedHeight + (SPACER_SIZE*G_SCALE)*2 );

    // update scroll bar
    double sf = static_cast<double>(restrictedHeight)/(allButtonsHeight*G_SCALE);
    scrollBar_->setHeight(restrictedHeight);
    scrollBar_->setGeometry(buttonWidth - 2, topMargin(), 10, restrictedHeight);
    scrollBar_->setBarPortion(sf);
    double yPosPercent = static_cast<double>(menuListPosY_) / menuListWidget_->geometry().height();
    scrollBar_->moveBarToPercentPos(yPosPercent);

    if (sf < .97) // protect from rounding errors
    {
        scrollBar_->show();
    }
    else
    {
        scrollBar_->hide();
    }

    // update main view port
    emit sizeChanged(buttonWidth  + sideMargin()*2, restrictedHeight + topMargin()*2 );
}

int ComboMenuWidget::largestButtonWidth()
{
    int largestButtonWidth = 0;

    for (auto button : items_)
    {
        if (button->widthUnscaled() > largestButtonWidth)
        {
            largestButtonWidth = button->widthUnscaled();
        }
    }

    return largestButtonWidth;
}


}