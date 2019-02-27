#ifndef THEMESMANAGER_H
#define THEMESMANAGER_H

#include <QString>

namespace KV {

class ThemesManager {
public:
    static void LoadTheme(const char *themes_path, const char *theme_name);
    static void ClearTheme();
    static char** GetPaths(int *count);
    static char** GetThemes(const char *themes_path, int *count);
    static const QString& Current();
    static const QString& Default();

private:
    static ThemesManager* instance();
    ThemesManager();
    ThemesManager(const ThemesManager&);
    ThemesManager& operator=(const ThemesManager&);

    void loadTheme(const QString& theme_name, const QString& theme_path);
    QStringList getPaths() const;
    QStringList getThemes(const QString& themes_path) const;

private:

    QString m_Current;
    QString m_Default;
    QString m_Template;

};


}

#endif // THEMESMANAGER_H
