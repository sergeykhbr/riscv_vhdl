/****************************************************************************
** Meta object code from reading C++ file 'RegsViewWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RegsViewWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_debugger__MyQMdiSubWindow_t {
    QByteArrayData data[5];
    char stringdata0[57];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_debugger__MyQMdiSubWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_debugger__MyQMdiSubWindow_t qt_meta_stringdata_debugger__MyQMdiSubWindow = {
    {
QT_MOC_LITERAL(0, 0, 25), // "debugger::MyQMdiSubWindow"
QT_MOC_LITERAL(1, 26, 13), // "signalVisible"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 11), // "slotVisible"
QT_MOC_LITERAL(4, 53, 3) // "val"

    },
    "debugger::MyQMdiSubWindow\0signalVisible\0"
    "\0slotVisible\0val"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_debugger__MyQMdiSubWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    1,   27,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    4,

       0        // eod
};

void debugger::MyQMdiSubWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MyQMdiSubWindow *_t = static_cast<MyQMdiSubWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->signalVisible((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->slotVisible((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (MyQMdiSubWindow::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&MyQMdiSubWindow::signalVisible)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject debugger::MyQMdiSubWindow::staticMetaObject = {
    { &QMdiSubWindow::staticMetaObject, qt_meta_stringdata_debugger__MyQMdiSubWindow.data,
      qt_meta_data_debugger__MyQMdiSubWindow,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *debugger::MyQMdiSubWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *debugger::MyQMdiSubWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_debugger__MyQMdiSubWindow.stringdata0))
        return static_cast<void*>(const_cast< MyQMdiSubWindow*>(this));
    return QMdiSubWindow::qt_metacast(_clname);
}

int debugger::MyQMdiSubWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMdiSubWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void debugger::MyQMdiSubWindow::signalVisible(bool _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_debugger__RegsViewWidget_t {
    QByteArrayData data[6];
    char stringdata0[77];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_debugger__RegsViewWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_debugger__RegsViewWidget_t qt_meta_stringdata_debugger__RegsViewWidget = {
    {
QT_MOC_LITERAL(0, 0, 24), // "debugger::RegsViewWidget"
QT_MOC_LITERAL(1, 25, 13), // "slotConfigure"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 14), // "AttributeType*"
QT_MOC_LITERAL(4, 55, 3), // "cfg"
QT_MOC_LITERAL(5, 59, 17) // "slotPollingUpdate"

    },
    "debugger::RegsViewWidget\0slotConfigure\0"
    "\0AttributeType*\0cfg\0slotPollingUpdate"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_debugger__RegsViewWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x08 /* Private */,
       5,    0,   27,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,

       0        // eod
};

void debugger::RegsViewWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        RegsViewWidget *_t = static_cast<RegsViewWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->slotConfigure((*reinterpret_cast< AttributeType*(*)>(_a[1]))); break;
        case 1: _t->slotPollingUpdate(); break;
        default: ;
        }
    }
}

const QMetaObject debugger::RegsViewWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_debugger__RegsViewWidget.data,
      qt_meta_data_debugger__RegsViewWidget,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *debugger::RegsViewWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *debugger::RegsViewWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_debugger__RegsViewWidget.stringdata0))
        return static_cast<void*>(const_cast< RegsViewWidget*>(this));
    if (!strcmp(_clname, "IGuiCmdHandler"))
        return static_cast< IGuiCmdHandler*>(const_cast< RegsViewWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int debugger::RegsViewWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
