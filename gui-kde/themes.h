#ifndef THEMES_H
#define THEMES_H

#include <QString>

namespace KV {

class Themes {
public:
    static void LoadTheme(const char *themes_path, const char *theme_name);
    static void ClearTheme();
    static char** GetPaths(int *count);
    static char** GetThemes(const char *themes_path, int *count);
    static const QString& Current();
    static const QString& Default();

private:
    static Themes* instance();
    Themes();
    Themes(const Themes&);
    Themes& operator=(const Themes&);

    void loadTheme(const QString& theme_name, const QString& theme_path);
    QStringList getPaths() const;
    QStringList getThemes(const QString& themes_path) const;

private:

    QString m_Current;
    QString m_Default;
    QString m_Template;

};


}

#endif // THEMES_H
