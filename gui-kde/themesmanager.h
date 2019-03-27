#ifndef THEMESMANAGER_H
#define THEMESMANAGER_H

#include <QString>
#include <QObject>

class QFileSystemWatcher;

namespace KV {

class ThemesManager: public QObject {
  Q_OBJECT
public:
    static void LoadTheme(const char *themes_path, const char *theme_name);
    static void ClearTheme();
    static char** GetPaths(int *count);
    static char** GetThemes(const char *themes_path, int *count);
    static const QString& Current();
    static const QString& Default();

private:
    static ThemesManager* instance();
    ThemesManager(QObject* parent = nullptr);
    ThemesManager(const ThemesManager&);
    ThemesManager& operator=(const ThemesManager&);

    void loadTheme(const QString& theme_name, const QString& theme_path);
    QStringList getPaths() const;
    QStringList getThemes(const QString& themes_path) const;

private slots:

    void reloadStyle(const QString& path);

private:

    QString m_Current;
    QString m_path;
    QString m_Default;
    QString m_Template;
    QFileSystemWatcher* m_styleWatcher = nullptr;

};


}

#endif // THEMESMANAGER_H
