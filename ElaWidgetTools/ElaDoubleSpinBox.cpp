#include "ElaDoubleSpinBox.h"

#include "DeveloperComponents/ElaSpinBoxStyle.h"
#include "ElaDoubleSpinBoxPrivate.h"
#include "ElaMenu.h"
#include "ElaTheme.h"

#include <QContextMenuEvent>
#include <QLineEdit>
#include <QPainter>
#include <QPropertyAnimation>
ElaDoubleSpinBox::ElaDoubleSpinBox(QWidget* parent)
    : QDoubleSpinBox(parent), d_ptr(new ElaDoubleSpinBoxPrivate())
{
    Q_D(ElaDoubleSpinBox);
    d->q_ptr = this;
    d->_pExpandMarkWidth = 0;
    setFixedSize(115, 35);
    d->_style = new ElaSpinBoxStyle(style());
    setStyle(d->_style);
    lineEdit()->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lineEdit()->setStyleSheet("background-color:transparent;padding-left:10px;padding-bottom:3px;");
    d->onThemeChanged(eTheme->getThemeMode());
    connect(eTheme, &ElaTheme::themeModeChanged, d, &ElaDoubleSpinBoxPrivate::onThemeChanged);
}

ElaDoubleSpinBox::~ElaDoubleSpinBox()
{
}

void ElaDoubleSpinBox::setButtonMode(ElaSpinBoxType::ButtonMode buttonMode)
{
    Q_D(ElaDoubleSpinBox);
    if (minimumWidth() < 90)
    {
        setMinimumWidth(90);
    }
    d->_style->setButtonMode(buttonMode);
    switch (buttonMode)
    {
    case ElaSpinBoxType::Inline:
    {
        lineEdit()->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        lineEdit()->setStyleSheet("background-color:transparent;padding-left:10px;padding-bottom:3px;");
        break;
    }
    case ElaSpinBoxType::Compact:
    case ElaSpinBoxType::Side:
    case ElaSpinBoxType::PMSide:
    {
        lineEdit()->setAlignment(Qt::AlignCenter);
        lineEdit()->setStyleSheet("background-color:transparent;padding-bottom:3px;");
        break;
    }
    }
    d->onThemeChanged(eTheme->getThemeMode());
    setFrame(hasFrame());
    Q_EMIT pButtonModeChanged();
}

ElaSpinBoxType::ButtonMode ElaDoubleSpinBox::getButtonMode() const
{
    Q_D(const ElaDoubleSpinBox);
    return d->_style->getButtonMode();
}

void ElaDoubleSpinBox::focusInEvent(QFocusEvent* event)
{
    Q_D(ElaDoubleSpinBox);
    if (event->reason() == Qt::MouseFocusReason)
    {
        QPropertyAnimation* markAnimation = new QPropertyAnimation(d, "pExpandMarkWidth");
        connect(markAnimation, &QPropertyAnimation::valueChanged, this, [=](const QVariant& value) {
            update();
        });
        markAnimation->setDuration(300);
        markAnimation->setEasingCurve(QEasingCurve::InOutSine);
        markAnimation->setStartValue(d->_pExpandMarkWidth);
        markAnimation->setEndValue(width() / 2 - 3);
        markAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    }
    QDoubleSpinBox::focusInEvent(event);
}

void ElaDoubleSpinBox::focusOutEvent(QFocusEvent* event)
{
    Q_D(ElaDoubleSpinBox);
    if (event->reason() != Qt::PopupFocusReason)
    {
        QPropertyAnimation* markAnimation = new QPropertyAnimation(d, "pExpandMarkWidth");
        connect(markAnimation, &QPropertyAnimation::valueChanged, this, [=](const QVariant& value) {
            update();
        });
        markAnimation->setDuration(300);
        markAnimation->setEasingCurve(QEasingCurve::InOutSine);
        markAnimation->setStartValue(d->_pExpandMarkWidth);
        markAnimation->setEndValue(0);
        markAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    }
    QDoubleSpinBox::focusOutEvent(event);
}

void ElaDoubleSpinBox::paintEvent(QPaintEvent* event)
{
    Q_D(ElaDoubleSpinBox);
    QDoubleSpinBox::paintEvent(event);
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(ElaThemeColor(d->_themeMode, PrimaryNormal));
    painter.drawRoundedRect(QRectF(width() / 2 - d->_pExpandMarkWidth, height() - 2.5, d->_pExpandMarkWidth * 2, 2.5), 2, 2);
    painter.restore();
}

void ElaDoubleSpinBox::contextMenuEvent(QContextMenuEvent* event)
{
    Q_D(ElaDoubleSpinBox);
    ElaMenu* menu = d->_createStandardContextMenu();
    if (!menu)
    {
        return;
    }
    menu->addSeparator();
    const uint se = stepEnabled();
    QAction* up = menu->addElaIconAction(ElaIconType::Plus, tr("增加"));
    up->setEnabled(se & StepUpEnabled);
    QAction* down = menu->addElaIconAction(ElaIconType::Minus, tr("减少"));
    down->setEnabled(se & StepDownEnabled);
    menu->addSeparator();

    const QAbstractSpinBox* that = this;
    const QPoint pos = (event->reason() == QContextMenuEvent::Mouse)
        ? event->globalPos()
        : mapToGlobal(QPoint(event->pos().x(), 0)) + QPoint(width() / 2, height() / 2);
    const QAction* action = menu->exec(pos);
    delete menu;
    if (that && action)
    {
        if (action == up)
        {
            stepBy(1);
        }
        else if (action == down)
        {
            stepBy(-1);
        }
    }
    event->accept();
}
