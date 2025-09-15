JVMTI Class File Dumper via Agent — Инструкция (Windows + Visual Studio)
📌 Описание

Агент JVMTI на C++, позволяющий дампить все загружаемые в JVM классы во время работы программы.
Сохраняет .class файлы в правильной структуре директорий.

🔧 Компиляция (Visual Studio)

Откройте Developer Command Prompt for VS

Установите переменную JAVA_HOME:

set JAVA_HOME=C:\Program Files\Java\jdk-17


Скомпилируйте DLL:

cl /LD classdumper.cpp /I"%JAVA_HOME%\include" /I"%JAVA_HOME%\include\win32" /Feclassdumper.dll

▶ Использование
Запуск Java-приложения с агентом:
java -javaagent:classdumper.dll=.\output -jar yourapp.jar


Если путь длинный — используйте кавычки:

java -javaagent:"C:\path\to\classdumper.dll"="C:\path\to\output" -jar yourapp.jar

⚙ Параметры агента

Если директория не указана → по умолчанию .\dumped_classes\

Пример:

java -javaagent:classdumper.dll=.\spring_classes -jar myapp.jar

🐞 Устранение неполадок

Ошибка Failed to get JVMTI environment
→ используйте JDK (а не JRE), совпадение разрядности (x64/x86).

Агент не сохраняет файлы
→ проверьте права на директорию, используйте полный путь.

Ошибка загрузки DLL
→ проверьте зависимости:

dumpbin /dependents classdumper.dll

🔍 Отладка

Подробный вывод загрузки классов:

java -XX:+TraceClassLoading -javaagent:classdumper.dll=.\output -jar yourapp.jar
