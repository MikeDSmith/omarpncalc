#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>
#include <QVector>

#include <functional>

// The RPN engine. The stack is unbounded (HP-48 style): stack.last() is X
// (level 1) and nothing is lost when pushing deep. A number being typed lives
// in `entry` until an operation commits it.
class Calculator : public QObject {
    Q_OBJECT
    // Four display rows, level 4 first: { level, value (text), typing }.
    Q_PROPERTY(QVariantList levels READ levels NOTIFY changed)
    Q_PROPERTY(QString message READ message NOTIFY messageChanged)
    Q_PROPERTY(bool clearArmed READ clearArmed NOTIFY changed)
    Q_PROPERTY(bool memoryHeld READ memoryHeld NOTIFY changed)
    Q_PROPERTY(bool empty READ isEmpty NOTIFY changed)

public:
    explicit Calculator(QObject *parent = nullptr);

    // Display format: up to 12 significant digits, never wider than 16
    // characters, exponent notation only when fixed would be misleading.
    static QString formatNumber(double value);

    QVariantList levels() const;
    QString message() const { return m_message; }
    bool clearArmed() const { return m_clearArmed; }
    bool memoryHeld() const { return m_memory != 0; }
    bool isEmpty() const { return m_stack.isEmpty() && m_entry.isEmpty(); }

    // Introspection for tests.
    const QVector<double> &stack() const { return m_stack; }
    QString entry() const { return m_entry; }
    double memory() const { return m_memory; }
    bool liftDisabled() const { return m_noLift; }

    Q_INVOKABLE void digit(const QString &digit);
    Q_INVOKABLE void backspace();
    Q_INVOKABLE void enter();
    Q_INVOKABLE void drop();
    Q_INVOKABLE void swap();
    Q_INVOKABLE void rollDown();
    Q_INVOKABLE void rollUp();
    Q_INVOKABLE void negate();
    Q_INVOKABLE void add();
    Q_INVOKABLE void subtract();
    Q_INVOKABLE void multiply();
    Q_INVOKABLE void divide();
    Q_INVOKABLE void reciprocal();
    Q_INVOKABLE void sqrt();
    // c: discard the entry, else zero X. Pressed again (while armed): ac.
    Q_INVOKABLE void clearKey();
    // ac: clear the stack, the entry and memory.
    Q_INVOKABLE void clearAll();
    Q_INVOKABLE void memoryRecall();
    Q_INVOKABLE void memoryAdd(double sign);
    Q_INVOKABLE void copy();
    Q_INVOKABLE void paste();
    Q_INVOKABLE void pushNumber(double value);
    Q_INVOKABLE bool pasteText(const QString &text);

signals:
    void changed();
    void messageChanged();

private:
    void commit();
    void lift();
    void push(double value);
    void binary(const std::function<double(double, double)> &op);
    void unary(const std::function<double(double)> &op);
    void finish();
    void fail(const QString &message);
    void flash(const QString &message);

    QVector<double> m_stack;
    QString m_entry;
    // After c zeroes X the 0 is a placeholder: the next value typed or pushed
    // replaces it instead of lifting the stack (HP "stack lift disabled").
    bool m_noLift = false;
    bool m_clearArmed = false;
    double m_memory = 0;
    QString m_message;
    QTimer m_flash;
};
