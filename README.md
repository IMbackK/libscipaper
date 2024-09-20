# libscipaper

libscipaper is a shared library that allows you to get metadata to scientific papers from various database services.

When queried libscipaper will ask eatch of its backends to the database services to try and fill in information about the requested paper in order of the priority given in scipaper.ini or in the order requested by the user of the User API.

libscipaper also allows you to get full text as well as pdf files for scientific papers where they are availble.

libscipaper also includes a cli utility, scipaper_cli, which allows you to: get referances to papers as json files or biblatex entries

* Get on publications metadata as a json file or as a biblatex entry
* Bulk Download pdfs of papers based on querys

For questions or comments, as well as help with the usage of the plugin API contact devnull@uvos.xyz

Full documentation is avialable at [here](https://uvos.xyz/kiss/libscipaperdoc)
