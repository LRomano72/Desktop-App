#include "custommenulineedit.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QStyle>
#include "graphicresources/fontmanager.h"

CustomMenuLineEdit::CustomMenuLineEdit(QWidget *parent) : BlockableQLineEdit (parent)
{
    setContextMenuPolicy(Qt::NoContextMenu);

    menu_ = new CustomMenuWidget();
    menu_->setColorScheme(false);
    menu_->initContextMenu();
    connect(menu_, SIGNAL(triggered(QAction*)), SLOT(onMenuTriggered(QAction*)));

}

void CustomMenuLineEdit::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        updateActionsState();
        menu_->exec(QCursor::pos());
    }
    else
    {
        BlockableQLineEdit::mousePressEvent(event);
    }
}

void CustomMenuLineEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Tab)
    {
        emit keyPressed(event);
    }
    else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        emit keyPressed(event);
    }
    else if (event->key() == Qt::Key_Escape)
    {
        emit keyPressed(event);
    }
    else
    {
        BlockableQLineEdit::keyPressEvent(event);
    }
}

void CustomMenuLineEdit::changeEvent(QEvent *event)
{
    if (event != nullptr && event->type() == QEvent::LanguageChange)
    {
        menu_->clearItems();
        menu_->clear();
        menu_->initContextMenu();
    }
    else
    {
        BlockableQLineEdit::changeEvent(event);
    }
}

void CustomMenuLineEdit::setColorScheme(bool darkMode)
{
    menu_->setColorScheme(darkMode);
}

void CustomMenuLineEdit::updateScaling()
{
    menu_->clearItems();
    menu_->clear();
    menu_->initContextMenu();
}

void CustomMenuLineEdit::onMenuTriggered(QAction *action)
{
    QVariant newItem = action->data();

    if (newItem.toInt() == CustomMenuWidget::UNDO)
    {
        undo();
    }
    else if (newItem.toInt() == CustomMenuWidget::REDO)
    {
        redo();
    }
    else if (newItem.toInt() == CustomMenuWidget::CUT)
    {
        cut();
    }
    else if (newItem.toInt() == CustomMenuWidget::COPY)
    {
        copy();
    }
    else if (newItem.toInt() == CustomMenuWidget::PASTE)
    {
        paste();
    }
    else if (newItem.toInt() == CustomMenuWidget::DELETE)
    {
        insert(""); // deletes selected text
    }
    else if (newItem.toInt() == CustomMenuWidget::SELECT_ALL)
    {
        selectAll();
    }

    emit itemClicked(action->text(), newItem);
}

void CustomMenuLineEdit::updateActionsState()
{
    foreach (QAction * action, menu_->actions())
    {
        QVariant userValue = action->data();

        bool enable = false;
        switch(userValue.toInt())
        {
        case CustomMenuWidget::UNDO:
            if (isUndoAvailable()) enable = true;
            break;
        case CustomMenuWidget::REDO:
            if (isRedoAvailable()) enable = true;
            break;
        case CustomMenuWidget::CUT:
        case CustomMenuWidget::COPY:
        case CustomMenuWidget::DELETE:
            if (hasSelectedText()) enable = true;
            break;
        case CustomMenuWidget::PASTE:
        {
            const QMimeData *mimeData = QApplication::clipboard()->mimeData();
            if (mimeData->hasText()) enable = true;
            break;
        }
        case CustomMenuWidget::SELECT_ALL:
            enable = true;
            break;
        }

        action->setEnabled(enable);
    }
}
