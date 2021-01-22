#ifndef DISPLAYCONFIG_H
#define DISPLAYCONFIG_H

#include <QObject>

class DisplayConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int statusBarHeight READ statusBarHeight NOTIFY statusBarHeightChanged)
    Q_PROPERTY(int navigationBarHeight READ navigationBarHeight NOTIFY navigationBarHeightChanged)
    Q_PROPERTY(int navigationBarHeightLandscape READ navigationBarHeightLandscape NOTIFY navigationBarHeightLandscapeChanged)
    Q_PROPERTY(int dpi READ dpi WRITE setDpi NOTIFY dpiChanged)
    Q_PROPERTY(float dp READ dp WRITE setDp NOTIFY dpChanged)
    Q_PROPERTY(float density READ density NOTIFY densityChanged)
    Q_PROPERTY(bool isTablet READ isTablet NOTIFY isTabletChanged)
    Q_PROPERTY(bool navBarVisible READ navBarVisible NOTIFY navBarVisibleChanged)

public:
    explicit DisplayConfig(QObject *parent = 0);

    Q_INVOKABLE void updateDisplayConfig();

    float dp() const;
    void setDp(float dp);

    float density() const;
    void setDensity(float density);

    int dpi() const;
    void setDpi(int dpi);

    int statusBarHeight() const;

    int navigationBarHeight() const;

    int navigationBarHeightLandscape() const;

    bool isTablet() const;
    void setIsTablet(bool isTablet);

    bool navBarVisible() const;

signals:
    void dpChanged();
    void dpiChanged();
    void statusBarHeightChanged();
    void navigationBarHeightChanged();
    void navigationBarHeightLandscapeChanged();
    void isTabletChanged();
    void navBarVisibleChanged();

    void densityChanged(float density);

private:
    float m_dp;
    float m_density;
    int m_dpi;
    int m_statusBarHeight;
    int m_navigationBarHeight;
    int m_navigationBarHeightLandscape;
    bool m_isTablet;
    bool m_navBarVisible;

    float displayDensity();
    int displayDpi();
    int getResourceSize(const QString &resource);

    bool isTablet();

    void setStatusBarHeight(int statusBarHeight);
    void setNavigationBarHeight(int navigationBarHeight);
    void setNavigationBarHeightLandscape(int navigationBarHeightLandscape);
    void setNavBarVisible(bool visible);
};

#endif // DISPLAYCONFIG_H
