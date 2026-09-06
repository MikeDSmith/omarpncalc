#include "calculator.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QLocale>
#include <QRegularExpression>
#include <QVariantMap>

#include <cmath>

namespace {
constexpr int kDigits = 12;
constexpr int kMaxWidth = 16;

// Drop a mantissa's trailing zeros and shorten the exponent: "1.500e+05" -> "1.5e+5".
QString tidyExponent(QString text) {
    static const QRegularExpression zeros(QStringLiteral("\\.?0+e"));
    static const QRegularExpression leading(QStringLiteral("e([+-])0*(\\d)"));
    text.replace(zeros, QStringLiteral("e"));
    text.replace(leading, QStringLiteral("e\\1\\2"));
    return text;
}

QString stripFraction(QString text) {
    if (!text.contains(QLatin1Char('.')))
        return text;
    while (text.endsWith(QLatin1Char('0')))
        text.chop(1);
    if (text.endsWith(QLatin1Char('.')))
        text.chop(1);
    return text;
}
}

Calculator::Calculator(QObject *parent) : QObject(parent) {
    m_flash.setSingleShot(true);
    m_flash.setInterval(1200);
    connect(&m_flash, &QTimer::timeout, this, [this] {
        m_message.clear();
        emit messageChanged();
    });
}

QString Calculator::formatNumber(double value) {
    if (!std::isfinite(value))
        return QStringLiteral("Error");
    if (value == 0)
        return QStringLiteral("0");

    // Whole numbers are exact up to 15 digits; show them as typed.
    if (value == std::round(value) && std::fabs(value) < 1e15)
        return QString::number(value, 'f', 0);

    // Round to 12 significant digits first so floating-point noise never
    // shows (0.1 + 0.2 -> 0.3), then lay the rounded value out.
    const double rounded = QString::number(value, 'g', kDigits).toDouble();
    const int exponent = static_cast<int>(std::floor(std::log10(std::fabs(rounded))));

    if (exponent >= -6) {
        QString fixed = stripFraction(
            QString::number(rounded, 'f', std::max(0, kDigits - 1 - exponent)));
        const int intDigits = fixed.section(QLatin1Char('.'), 0, 0).remove(QLatin1Char('-')).size();
        // Fixed notation only when it shows every integer digit honestly: a
        // value like 1234567890123456 would otherwise print as 1234567890120000.
        if (fixed.size() <= kMaxWidth && intDigits <= kDigits)
            return fixed;
    }

    QString text;
    for (int digits = kDigits - 1; digits >= 0; --digits) {
        text = tidyExponent(QString::number(rounded, 'e', digits));
        if (text.size() <= kMaxWidth)
            return text;
    }
    return text;
}

QVariantList Calculator::levels() const {
    QVector<QString> all;
    all.reserve(m_stack.size() + 1);
    for (double v : m_stack)
        all.append(formatNumber(v));
    const bool typing = !m_entry.isEmpty();
    if (typing)
        all.append(m_entry);

    QVariantList rows;
    for (int level = 4; level >= 1; --level) {
        const int index = all.size() - level;
        QVariantMap row;
        row.insert(QStringLiteral("level"), level);
        row.insert(QStringLiteral("value"), index >= 0 ? all.at(index) : QString());
        row.insert(QStringLiteral("typing"), typing && level == 1);
        rows.append(row);
    }
    return rows;
}

// ---- state plumbing ----

void Calculator::finish() {
    m_clearArmed = false;
    emit changed();
}

void Calculator::flash(const QString &message) {
    m_message = message;
    m_flash.start();
    emit messageChanged();
}

// A refused operation leaves the stack (and any lift state) untouched.
void Calculator::fail(const QString &message) {
    flash(message);
    finish();
}

void Calculator::lift() {
    if (m_noLift && !m_stack.isEmpty())
        m_stack.removeLast();
    m_noLift = false;
}

// The value of the entry as typed so far. An exponent still being typed --
// "5e", or "5e-" after +/- -- is not yet a parseable number, and reading it as
// zero silently turned a half-finished 5.5e3 into 0 on the next operation.
// Fall back to the mantissa, which is what the entry means at that point.
double Calculator::entryValue() const {
    bool ok = false;
    double value = QLocale::c().toDouble(m_entry, &ok);
    if (ok)
        return value;

    const int exponent = m_entry.indexOf(QLatin1Char('e'));
    if (exponent > 0) {
        value = QLocale::c().toDouble(m_entry.left(exponent), &ok);
        if (ok)
            return value;
    }
    return 0;
}

// Push the pending entry onto the stack, if there is one.
void Calculator::commit() {
    if (m_entry.isEmpty())
        return;
    m_stack.append(entryValue());
    m_entry.clear();
}

void Calculator::push(double value) {
    commit();
    lift();
    m_stack.append(value);
}

// ---- entry ----

void Calculator::digit(const QString &d) {
    if (m_entry.isEmpty())
        lift();
    m_noLift = false;

    QString e = m_entry;
    if (d == QStringLiteral(".")) {
        if (e.contains(QLatin1Char('.')) || e.contains(QLatin1Char('e')))
            return finish();
        if (e.isEmpty() || e == QStringLiteral("-"))
            e += QLatin1Char('0');
    } else if (d == QStringLiteral("e")) {
        if (e.contains(QLatin1Char('e')))
            return finish();
        if (e.isEmpty() || e == QStringLiteral("-"))
            e += QLatin1Char('1');
    } else if (e == QStringLiteral("0")) {
        e.clear();
    } else if (e == QStringLiteral("-0")) {
        e = QStringLiteral("-");
    }
    m_entry = e + d;
    finish();
}

void Calculator::backspace() {
    if (!m_entry.isEmpty()) {
        m_entry.chop(1);
        return finish();
    }
    drop();
}

// ENTER pushes the typed value; with nothing typed it duplicates X.
void Calculator::enter() {
    if (!m_entry.isEmpty())
        commit();
    else if (!m_stack.isEmpty())
        m_stack.append(m_stack.last());
    m_noLift = false;
    finish();
}

// ---- stack ----

void Calculator::drop() {
    if (!m_entry.isEmpty())
        m_entry.clear();
    else if (!m_stack.isEmpty())
        m_stack.removeLast();
    m_noLift = false;
    finish();
}

void Calculator::clearAll() {
    m_stack.clear();
    m_entry.clear();
    m_noLift = false;
    m_memory = 0;
    finish();
}

void Calculator::clearKey() {
    if (m_clearArmed)
        return clearAll();

    if (!m_entry.isEmpty()) {
        m_entry.clear();
    } else if (!m_stack.isEmpty()) {
        m_stack.last() = 0;
        m_noLift = true;
    }
    finish();
    m_clearArmed = true;
    emit changed();
}

void Calculator::swap() {
    commit();
    if (m_stack.size() < 2)
        return fail(QStringLiteral("Need two values"));
    std::swap(m_stack[m_stack.size() - 1], m_stack[m_stack.size() - 2]);
    m_noLift = false;
    finish();
}

// Roll down: X moves to the bottom of the stack, everything else moves one
// level closer to X.
void Calculator::rollDown() {
    commit();
    if (m_stack.size() >= 2)
        m_stack.prepend(m_stack.takeLast());
    m_noLift = false;
    finish();
}

// Roll up: the bottom value becomes X.
void Calculator::rollUp() {
    commit();
    if (m_stack.size() >= 2)
        m_stack.append(m_stack.takeFirst());
    m_noLift = false;
    finish();
}

void Calculator::negate() {
    if (!m_entry.isEmpty()) {
        const int ei = m_entry.indexOf(QLatin1Char('e'));
        if (ei >= 0) {
            // Negate the exponent while typing it, like a real HP.
            QString exp = m_entry.mid(ei + 1);
            exp = exp.startsWith(QLatin1Char('-')) ? exp.mid(1) : QLatin1Char('-') + exp;
            m_entry = m_entry.left(ei + 1) + exp;
        } else if (m_entry.startsWith(QLatin1Char('-'))) {
            m_entry.remove(0, 1);
        } else {
            m_entry.prepend(QLatin1Char('-'));
        }
        return finish();
    }
    if (!m_stack.isEmpty())
        m_stack.last() = -m_stack.last();
    m_noLift = false;
    finish();
}

// ---- arithmetic ----

void Calculator::binary(const std::function<double(double, double)> &op) {
    commit();
    if (m_stack.size() < 2)
        return fail(QStringLiteral("Need two values"));
    const double x = m_stack.last();
    const double y = m_stack.at(m_stack.size() - 2);
    const double r = op(y, x);
    if (!std::isfinite(r))
        return fail(QStringLiteral("Error"));
    m_stack.removeLast();
    m_stack.last() = r;
    m_noLift = false;
    finish();
}

void Calculator::unary(const std::function<double(double)> &op) {
    commit();
    if (m_stack.isEmpty())
        return fail(QStringLiteral("Need a value"));
    const double r = op(m_stack.last());
    if (!std::isfinite(r))
        return fail(QStringLiteral("Error"));
    m_stack.last() = r;
    m_noLift = false;
    finish();
}

void Calculator::add()        { binary([](double y, double x) { return y + x; }); }
void Calculator::subtract()   { binary([](double y, double x) { return y - x; }); }
void Calculator::multiply()   { binary([](double y, double x) { return y * x; }); }
void Calculator::divide()     { binary([](double y, double x) { return y / x; }); }
void Calculator::reciprocal() { unary([](double x) { return 1 / x; }); }
void Calculator::sqrt()       { unary([](double x) { return std::sqrt(x); }); }

// ---- memory ----

void Calculator::memoryRecall() {
    pushNumber(m_memory);
}

void Calculator::memoryAdd(double sign) {
    commit();
    if (!m_stack.isEmpty())
        m_memory += sign * m_stack.last();
    finish();
}

// ---- clipboard ----

void Calculator::pushNumber(double value) {
    push(value);
    finish();
}

void Calculator::copy() {
    // Copy the value, not the keystrokes, so what lands on the clipboard is
    // always a number that pastes back.
    QString text;
    if (!m_entry.isEmpty())
        text = formatNumber(entryValue());
    else if (!m_stack.isEmpty())
        text = formatNumber(m_stack.last());
    else
        return;
    if (QClipboard *clipboard = QGuiApplication::clipboard())
        clipboard->setText(text);
    flash(QStringLiteral("Copied"));
}

// Push a number from text, tolerating whitespace and thousands separators --
// both the comma kind and the space kind.
bool Calculator::pasteText(const QString &text) {
    static const QRegularExpression space(QStringLiteral("\\s"));
    QString cleaned = text.trimmed();
    cleaned.remove(space);
    cleaned.remove(QLatin1Char(','));
    bool ok = false;
    const double value = QLocale::c().toDouble(cleaned, &ok);
    if (!ok || !std::isfinite(value)) {
        fail(QStringLiteral("Not a number"));
        return false;
    }
    pushNumber(value);
    return true;
}

void Calculator::paste() {
    if (const QClipboard *clipboard = QGuiApplication::clipboard())
        pasteText(clipboard->text());
}
