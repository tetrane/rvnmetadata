# rvnmetadata

## What's this

This library is meant to be used by resource writers and readers to map the raw metadata with meaningful information.

It can also read the metadata from the supported resources directly.


## What are metadata

Metadata are compose of 5 fields:

* **Resource type**: enum class representing the type of the resource.

* **Version**: class representing the semantic versioning 2.0 of the resource. It is composed of numbers major, minor and patch which represent a version of the form X.X.X (like 1.2.0) and can have prerelease and build identifiers. It is mainly used to know what is compatible with this resource.

* **Tool name**: simple string which contains the name of the tool that has generated the resource.

* **Tool info**: simple string which gives information about the tool that has generated the resource, like the plugin used or the version of the tool.

* **generation date**: time point when the resource has been generated.


## How to version a resource (with supported type like sqlite or binary)

External libs like `rvnsqlite` and `rvnbinresource` bring support for sqlite databases and binary file formats respectively.
These libs provides wrappers around `sqlite::Database` (`ResourceDatabase`) and `ostream`/`istream` (`rvnbinresource::Writer`/`rvnbinresource::Reader`),
which take care of creating/opening these file types and writing to/fetching from them.

##### Writing:
Instantiate `Metadata` from constructor of `metadata.h` and pass it to wrapper

As a convention, you should define c-string variables `format_version` and `writer_version` and make them visible (obvious header) to ease updates, commonly like this:
```cpp
constexpr const char* writer_version = "1.0.0";
```

##### Reading:
Once a wrapper object is instanciated, you are responsible for checking version and resource type validity before trying to read its content.

Because of that, it is recommended to provide a method to retrieve semantic metadata from wrapper's raw_metadata, using the `rvnmetadata` helper functions (such as `from_raw_metadata`).


#### Tests:

* Since users are responsible for correct handling of the metadata, you should update your tests to check that metadata are correctly written, read & used.


## How to use metadata binaries

There are 2 metadata binaries: `metadata_reader` and `metadata_writer` located in `{OUTPUT_DIR}/share/reven/bin` and `{OUTPUT_DIR}/share/reven/internals` respectively.

The `metadata_reader` helps the user to reader metadata from a file.
The `metadata_writer` helps the user overwrite metadata with new ones.

Both have options to specify what metadata they want to read or write.
With the reader the user can also specify the output format: text or json

e.g:

`./metadata_reader {MY_VERSIONNED_FILE} --output=json --version`

--> `{ "version"="1.2.0-dev" }`

`./metadata_writer {MY_VERSIONNED_FILE} --version 1.3.0-release`

`./metadata_reader {MY_VERSIONNED_FILE} --output=text --version`

--> `version: 1.3.0-release`
