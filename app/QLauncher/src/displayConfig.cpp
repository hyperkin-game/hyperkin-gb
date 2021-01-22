#include "displayConfig.h"

#include <QScreen>
#include <QGuiApplication>
#include <QDebug>

DisplayConfig::DisplayConfig(QObject *parent) :
    QObject(parent),
    m_dpi(0),
    m_isTablet(false)
{
    m_dpi = displayDpi();
    m_density = displayDensity();

    m_dp = (float) m_dpi / 160;
    //qDebug() << "m_dpi="<<m_dpi<<",m_density="<<m_density;
    setStatusBarHeight(getResourceSize("status_bar_height"));
    setNavigationBarHeight(getResourceSize("navigation_bar_height"));
    setNavigationBarHeightLandscape(getResourceSize("navigation_bar_height_landscape"));

    updateDisplayConfig();

    m_isTablet = isTablet();
}

void DisplayConfig::updateDisplayConfig()
{
}

int DisplayConfig::dpi() const
{
    return m_dpi;
}

void DisplayConfig::setDpi(int dpi)
{
    if (m_dpi == dpi)
        return;

    m_dpi = dpi;
    emit dpiChanged();

    setDp((float) m_dpi / 160);
}

float DisplayConfig::dp() const
{
    return m_dp;
}

void DisplayConfig::setDp(float dp)
{
    if (m_dp == dp)
        return;

    m_dp = dp;
    emit dpChanged();
}

bool DisplayConfig::isTablet() const
{
    return m_isTablet;
}

void DisplayConfig::setIsTablet(bool isTablet)
{
    if (m_isTablet == isTablet)
        return;

    m_isTablet = isTablet;
    emit isTabletChanged();
}

bool DisplayConfig::navBarVisible() const
{
    return m_navBarVisible;
}

float DisplayConfig::density() const
{
    return m_density;
}

void DisplayConfig::setDensity(float density)
{
    if (m_density == density)
        return;

    m_density = density;
    emit densityChanged(density);
}

int DisplayConfig::statusBarHeight() const
{
    return m_statusBarHeight;
}

int DisplayConfig::navigationBarHeight() const
{
    return m_navigationBarHeight;
}

int DisplayConfig::navigationBarHeightLandscape() const
{
    return m_navigationBarHeightLandscape;
}

void DisplayConfig::setStatusBarHeight(int statusBarHeight)
{
    if (m_statusBarHeight == statusBarHeight)
        return;

    m_statusBarHeight = statusBarHeight;
    emit statusBarHeightChanged();
}

void DisplayConfig::setNavigationBarHeight(int navigationBarHeight)
{
    if (m_navigationBarHeight == navigationBarHeight)
        return;

    m_navigationBarHeight = navigationBarHeight;
    emit navigationBarHeightChanged();
}

void DisplayConfig::setNavigationBarHeightLandscape(int navigationBarHeightLandscape)
{
    if (m_navigationBarHeightLandscape == navigationBarHeightLandscape)
        return;

    m_navigationBarHeightLandscape = navigationBarHeightLandscape;
    emit navigationBarHeightLandscapeChanged();
}

void DisplayConfig::setNavBarVisible(bool visible)
{
    if (m_navBarVisible == visible)
        return;

    m_navBarVisible = visible;
    emit navBarVisibleChanged();
}

bool DisplayConfig::isTablet()
{
    return false;
}

int DisplayConfig::displayDpi()
{
    int dpi=QGuiApplication::primaryScreen()->physicalDotsPerInch();
    if(dpi<120 || dpi >640)
    {
        dpi=160;
         qDebug() << "displayDpi may read wrong value,let's set to 160";
    }
    return dpi;
}

float DisplayConfig::displayDensity()
{
    int density=QGuiApplication::primaryScreen()->physicalDotsPerInch();
    if(density<120 || density >640)
    {
        density=160;
        qDebug() << "displayDensity may read wrong value,let's set to 160";
    }
    return density;
}

int DisplayConfig::getResourceSize(const QString &resource)
{
    Q_UNUSED(resource);
    return 0;
}
