---
name: Feature request
about: Suggest an idea for this project
title: '[FEAT] '
labels: enhancement
assignees: ''

---

## Before requesting a new feature

* A lot of features are not enabled by default. Please try `fastfetch --list-modules` and `fastfetch -c all.jsonc` to see if it has been supported
* Fastfetch supports `Command` module, which can be used to grab output from a custom shell script. Please check if it fits your needs

```jsonc
// ~/.config/fastfetch/fastfetch.jsonc
{
    "modules": [
        {
            "type": "command",
            "text": "/path/to/your/script",
            "key": "Feature Title"
        }
    ]
}
```

## Wanted features:

<!-- Your features here -->

## Motivation:

<!-- Your motivation here -->
