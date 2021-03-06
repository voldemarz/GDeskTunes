#ifndef MEDIAKEYS_H
#define MEDIAKEYS_H

#include <QObject>
#include <QKeyEvent>
#include <QDebug>

/**
 * This is a template class which makes your widget capable of receiving
 * the keyPressEvents of the the MediaKeySource.
 */
template <class T>
class MediaKeyTarget: public T
{
public:
    MediaKeyTarget(QWidget *parent = 0) : T(parent)
    {
    }
    virtual ~MediaKeyTarget()
    {
    }

public:
    void keyPressEvent(QKeyEvent *event)
    {
        T::keyPressEvent(event);
    }

};

/**
 * Use this class if you do not want to rely on the Qt Event handling
 * system to propagate Media Key events to the parent widget.
 *
 * The parent must implement a "public" keyPressEvent.
 * You can use MediaKeyTarget to implement this.
 */
template <class T, class Parent>
class MediaKeySource: public T
{
public:
    MediaKeySource(QWidget *parent = 0) : T(parent)
    {
        // Nothing to do here
    }
    virtual ~MediaKeySource()
    {
        // Nothing to do here
    }

public:
    void keyPressEvent(QKeyEvent *event)
    {
        switch(event->key())
        {
        case Qt::Key_MediaPlay:
        case Qt::Key_MediaNext:
        case Qt::Key_MediaPrevious:
        {
            Parent *w = qobject_cast<Parent*>(T::parentWidget());
            if (w != 0)
            {
                qDebug() << T::objectName() << " forwards media key";
                w->keyPressEvent(event);
                break;
            }
        }
        default:
            T::keyPressEvent(event);
            /* TODO: is this really needed ?
             * Events will be propagated to the parent,
             * according to the documentation of the Qt Documentation.
             *
            if (!event->isAccepted())
            {
                Parent *w = dynamic_cast<Parent*>(T::parentWidget());
                if (w != 0)
                {
                    qDebug() << "Forwarding keyPress: " << T::objectName() << event->key();
                    w->keyPressEvent(event);
                }
            }
            */
        }
    }
};

namespace Private
{
    class MediaKeySignal: public QObject
    {
        Q_OBJECT
    public:
        MediaKeySignal() {}

    signals:
        void keyPressed(QKeyEvent *event);

    };
}
template <class T>
class MediaKeySignal : public T, Private::MediaKeySignal
{
public:
    MediaKeySignal(QWidget *parent = 0) : T(parent)
    {
    }
    virtual ~MediaKeySignal()
    {
    }

public:
    void keyPressEvent(QKeyEvent *event)
    {
        switch(event->key())
        {
        case Qt::Key_MediaPlay:
        case Qt::Key_MediaNext:
        case Qt::Key_MediaPrevious:
        {
            qDebug() << T::objectName() << " signals media key";
            emit keyPressed(event);
            break;
        }
        default:
            qDebug() << "Key Pressed" << event;
            T::keyPressEvent(event);
            /* TODO: is this really needed ?
             * Events will be propagated to the parent,
             * according to the documentation of the Qt Documentation.
             *
            if (!event->isAccepted())
            {
                qDebug() << "emit keyPressed(" << event << ")";
                emit keyPressed(new QKeyEvent(event->type(), event->key(), event->modifiers()));
                event->ignore();
            }
            */
            break;
        }
    }
};

#endif // MEDIAKEYS_H
