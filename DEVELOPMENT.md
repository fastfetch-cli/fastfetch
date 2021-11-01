# Development help

This file is not an official documentation!
Here i just add things that are easy to forget.

## Adding a logo

Assuming foobar is the name of the logo:

In `src/common/logo.c`:
* create method `initLogoFoobar(FFinstance* instance)`, at alphabetic ordered placement. Set the fields of `instance->logo`:
    ```c
    instance->config.logo.lines =
        "$1Foo\n"
        "    $2bar";
    instance->config.logo.colors[0] = "\033[32m"; //green
    instance->config.logo.colors[1] = "\033[35m"; //magenta
    ```
* go to method `loadLogoSet` and add an if at alphabetical position to load it:
    ```c
    else if(strcasecmp(logo, "foobar") == 0)
        initLogoFoobar(instance);
    ```
* if different logos exist for different versions, differ betwenn them in `loadLogoSetWithVersion`:
    ```c
    [...]

    bool isFoobar = ffStrbufIgnCaseCompS(name, "foobar") == 0;

    if(versionID->length == 0 || (
        [...] &&
        !isFoobar
    )) return loadLogoSet(instance, name->chars);

    [...]

    if(isFoobar)
    {
        if(version > 64)
            return loadLogoSet(instance, "foobar");
        else
            return loadLogoSet(instance, "foobar_old");
    }
    ```
* go to `ffPrintLogos` and at `FF_LOGO_PRINT` at alphabetical position:
    ```c
    FF_LOGO_PRINT(foobar, initLogoFoobar)
    ```

In `src/data/logos.txt`:

* Add the name of the logo at alphabetical position.
   ```
   [...]
   foobar
   [...]
   ```

In `README.md`:

* Add the pretty name of the logo to the list of supported logos
    ```
    [...] Foobar, [...]
    ```
