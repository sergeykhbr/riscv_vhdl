/****************************************************************************
** Meta object code from reading C++ file 'ConsoleWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ConsoleWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_debugger__ConsoleWidget_t {
    QByteArrayData data[10];
    char stringdata0[133];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_debugger__ConsoleWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_debugger__ConsoleWidget_t qt_meta_stringdata_debugger__ConsoleWidget = {
    {
QT_MOC_LITERAL(0, 0, 23), // "debugger::ConsoleWidget"
QT_MOC_LITERAL(1, 24, 11), // "signalClose"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 8), // "QWidget*"
QT_MOC_LITERAL(4, 46, 14), // "AttributeType&"
QT_MOC_LITERAL(5, 61, 13), // "slotConfigure"
QT_MOC_LITERAL(6, 75, 14), // "AttributeType*"
QT_MOC_LITERAL(7, 90, 3), // "cfg"
QT_MOC_LITERAL(8, 94, 18), // "slotRepaintByTimer"
QT_MOC_LITERAL(9, 113, 19) // "slotClosingMainForm"

    },
    "debugger::ConsoleWidget\0signalClose\0"
    "\0QWidget*\0AttributeType&\0slotConfigure\0"
    "AttributeType*\0cfg\0slotRepaintByTimer\0"
    "slotClosingMainForm"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_debugger__ConsoleWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   34,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   39,    2, 0x08 /* Private */,
       8,    0,   42,    2, 0x08 /* Private */,
       9,    0,   43,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 4,    2,    2,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void debugger::ConsoleWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ConsoleWidget *_t = static_cast<ConsoleWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->signalClose((*reinterpret_cast< QWidget*(*)>(_a[1])),(*reinterpret_cast< AttributeType(*)>(_a[2]))); break;
        case 1: _t->slotConfigure((*reinterpret_cast< AttributeType*(*)>(_a[1]))); break;
        case 2: _t->slotRepaintByTimer(); break;
        case 3: _t->slotClosingMainForm(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (ConsoleWidget::*_t)(QWidget * , AttributeType & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ConsoleWidget::signalClose)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject debugger::ConsoleWidget::staticMetaObject = {
    { &QPlainTextEdit::staticMetaObject, qt_meta_stringdata_debugger__ConsoleWidget.data,
      qt_meta_data_debugger__ConsoleWidget,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *debugger::ConsoleWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *debugger::ConsoleWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_debugger__ConsoleWidget.stringdata0))
        return static_cast<void*>(const_cast< ConsoleWidget*>(this));
    if (!strcmp(_clname, "IConsole"))
        return static_cast< IConsole*>(const_cast< ConsoleWidget*>(this));
    return QPlainTextEdit::qt_metacast(_clname);
}

int debugger::ConsoleWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QPlainTextEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void debugger::ConsoleWidget::signalClose(QWidget * _t1, AttributeType & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
