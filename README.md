This is a live document. Email <info@tradery.com> with any questions or feedback.

# Tradery Engine

This project contains an updated version of the Tradery Engine (TE) used by the tradery.com (presently inactive) backend.

The purpose of this module is to execute user defined scripts for algorithmic trading systems, over specified market data (symbols, duration), and generate:

1. back-testing results such as simulated trades, performance statistics, charts etc used to evaluate the capabilities of a trading system
2. trading system optimization
2. trading signals, to be used for real-time trading

# Tradery.com
At the time tradery.com was active, it provided a complete trading system development environment, along with other features and services for traders and investors.

The TE used at that time was fully functional and stable. The current TE is based on that version, but has some important changes:

- has been ported to use recent C++ development tools (Visual Studio 2015)
- is ready to be ported to 64 bit
- has an additional Thrift API
- contains many internal improvements and fixes.

This new project is still work in progress, and although most of the code is functional, some features may not be available at full capacity.

## Objectives

Here is a list of initial design goals, and how they were achieved. These goals were decided in part based on past experience with other trading products, with the overall goal of 
fixing what seemed to be flaws or limitations in these products, and allowing new functionality that wasn't possible using them.

For instance, in order to build an online trading simulation platform, issues like performance, scalability, stability, massive concurrent use etc needed to be considered.

| |Objective | Achieved how |
|----|-------|--------------|
|1| High performance for real-time use |- written in C++<br>- multi-threading and multi-processing<br>- use of data caches<br>- compiled system code<br>  |
|2| Realistic simulation| support for predefined or user-definible parameters such as slippage, commission algorithms, position size, account size, position size limits etc|
|3| Support for trading systems from the most trivial to the most complex| C++ based system definition language<br>with many trading specific constructs|
|4| Support for various types of market data (e.g. bar, tick)| internal abstraction of data type|
|5| Support for trading on portfolios of stocks (diversification by asset)| |
|6| Support for trading using multi-systems (diversification by system)| |
|7| Able to handle very large datasets| efficient data and memory management|
|8| Easy to learn and use script-like language| abstraction of complex trading functions | |
|9| Support for a variety of common or more obscure statistical and technical indicators| use of TALib technical indicators library|
|10| Able to generate trading signals| |
|11| Able to generate statistics used to objectively assess the performance and risk of a trading system||
|12| Easy to use high level API| implementation of REST and Thrift APIs|

While there are many other programming languages out there, C++ was chosen for its flexibility, large number of third party 
libraries available for any concievable functionality, and not least, its ability to generate some 
of the most efficient code, on par with assembly language.

## TE Implementation

The TE is written entirely in C++ using Visual Studio, and runs as a Windows service.

## Source code

The Visual Studio 2015 "tradery" solution several projects with code for various modules and plugins - here's a brief description:

Here's a brief description of each project:

* **core** - schedules work between different plugins. Does multi-processing for increased performance.
* **datasource** - dll with code for traversing data files.
* **fileplugins** - dll with code for various plugins (datasource, symbolssource, statistics, charting, commission, slippage)
* **filesymbols** - static lib, code for handling symbols lists
* **include** - no target, container for common include files used across different projects
* **misc** - dll, code for various general utility classes and functions
* **miscwin** - dll, code for various windows oriented utility classes and functions
* **optimizer** - exe, code for running trading system optimization sessions, uses an older API and has not been maintained
* **plugin** - static library with plugin specific utility classes
* **runtimeproj** - used at runtime to generate "runnable" plugins containing user system code
* **systemsplugin1, 2** - dlls, contain unit test systems, used to ensure there are no system syntax and structure regressions
* **testthriftclient** - exe, contains test thrift API client, used for integration testing by making simulated API calls and verifying results.
* **thrift** - generates that C++ code from the API thrift definition file.
* **tradery** - exe, implements the service executable running the tradery thrift server listening for API calls, creates and controls sessions, and returns results. This is the main project of the solution.
* **traderyapi** - static lib, implements the old REST API, no longer maintained

## Source code formatting
All .h and .cpp source files have been formatted with the clang-format utility for linux using the following command line from the _src_ directory:

```
find .  -iname "*.h" -o -iname "*.cpp" | xargs clang-format-5.0 --style=file -i {}
```

The _src_ directory contains a hidden clang-format configuration file called .clang-format, which instructs it to 
use google style and to not sort #include directives (which would otherwise break the build, as certain #include directives must be in a specific order).

## Plugins

The TE uses plugin based architecture, where most of the work is performed by components (plugins) discovered and loaded at runtime, 
based on availability and configuration parameters.

Plugins are implemented as DLLs exporting certain APIs that allow them to be discovered and used.

Each plugin is identified by a UUID.

The work is coordinated by a core module, which receives commands via API calls, and distributes and schedules 
tasks to be performed to the different plugins.

Typical plugins:

- data source, which loads data to be processed,
- symbols source, which retrieves lists of symbols to be processed,
- runnable, which contains user code as compiled binary components.

Most plugins are statically generated, meaning they are compiled before hand, deployed, and loaded at runtime by the TE.

However, this cannot be the case with "runnable" plugins, which depend on code passed from the user via APIs at runtime: they to be dynamically *generated* and loaded .

The TE achieves this by using an internal runnable plugin generator which takes the user code, translates it into the complete plugin C++ code and project, which are then passed to the 
C++ compiler and linker to generate the plugin binary. This process is very fast thanks to a number of optimizations, 
and is definitely much faster than the total user code processing time in most cases.

During the execution of the user system (called a session), various output data is generated, which is stored as files in a directory associated with that sessions.

## Trading system definition language

Generally, most trading applications that allow users to define their own trading systems as scripts are based on interpreters of 
some popular or proprietary language. While easier to integrate into the product, most don't provide the level of flixibility, performance 
and reliability required to run complex systems over massive amounts of data in real-time.

The trading system definition language (TSDL) used by the TE is based on C++, athough with many simplifying primitives that should allow 
users of any programming skill level to write their own code.

The user code has a specific structure and syntax that is translated internally into C++ code, which is then compiled to generate 
the runnable binary.

Here's an example of a typical system:

``` C++
void run() {
  installTimeBasedExit(2);

  Series SMAclose2 = closeSeries().SMA(2);
  Series SMAclose5 = closeSeries().SMA(5);
  Series SMAclose30 = closeSeries().SMA(30);
  double lc = 0.93;
  double sc = 0.93;

  for (Index bar = 1; bar < barsCount(); bar++) {
    applyAutoStops(bar);

    for (Position pos = getFirstOpenPosition(); pos;
         pos = getNextOpenPosition()) {
      double posPrice = pos.getEntryPrice();

      if (open(bar + 1) > posPrice AND pos.isLong())
        sellAtMarket(bar + 1, pos, "Sell at Market");

      if (open(bar + 1) < posPrice AND pos.isShort())
        coverAtMarket(bar + 1, pos, "Cover at Market");
    }

    if (SMAclose5[bar] < lc * SMAclose30[bar]) {
      for (Index j = 1; j < 31; j++) {
        double lf = 0.75 - (j * 0.005);
        double lp = SMAclose2[bar] * lf;
        buyAtLimit(bar + 1, lp, 1000, "BuyAtLimit");
      }
    }

    if (SMAclose5[bar] > sc * SMAclose30[bar]) {
      for (Index j = 1; j < 31; j++) {
        double sf = 1.25 + (j * 0.005);
        double sp = SMAclose2[bar] * sf;
        shortAtLimit(bar + 1, sp, 1000, "ShortAtLimit");
      }
    }
  }
}

```

## Data
Market data consists of CSV files containing bar price and volume data for various symbols. The current data uses End Of Day (EOD) data, but intraday
data could be potentially used as well.

Each line consists of data in the following format:
```
Date,Open,High,Low,Close,Volume
```
where Date is in `mm/dd/yyyy` format.

Example:
```
9/27/2017,73.550003,74.169998,73.169998,73.849998,19565100
9/28/2017,73.540001,73.970001,73.309998,73.870003,10883800
9/29/2017,73.940002,74.540001,73.879997,74.489998,17079100
10/2/2017,74.709999,75.010002,74.300003,74.610001,15304800
10/3/2017,74.669998,74.879997,74.190002,74.260002,12190400
10/4/2017,74.089996,74.720001,73.709999,74.690002,13317700
```

Data needs to be dividend and split adjusted, although in principle, if dividend and split information could be provided separately, the data could 
also be unadjusted.

Data files are located under the directory specified in the configuration file. As the number of data files can be quite large, they are organized in a
directory tree structure, where the first level consists of directories named after the first character of the symbol, and the second level are directories named after the second
character of the symbol name (if any). This is an optimization that increases the access speed to data files by the datasource plugin.

For instance, if the symbols are MIB, MSB, MSFT, TI, TIBX, the structure is:
```
<data_dir>
    - M (d)
        - I (d)
            - MIB.csv (f)
        - S (d)
            - MSB.csv (f)
            - MSFT.csv (f)
    - T (d)
        - I (d)
            - TI.csv (f)
            - TIBX.csv (f)
```
## API

The original TE that ran on tradery.com used a shared pipe to receive calls from the PHP UI layer. These calls
were initiated by either user input or by calling a PHP REST API.

The C++ project "traderyapi" is an attempt at implementing the REST API directly in C++, but it is currently unmaintained.

The TE pipe is still active, although not currently used.

The current version of the TE uses a Thrift API, defined in the "thrift" project, and implemented in the "tradery" project.


## Build

Currently setting up the build environment is a manual process, but a script automating the entire build may be provided in the future.

Steps for setting up the build environment:

- Install Visual Studio 2015 with C++ options enabled
- Install or build various dependencies:
    - OpenSSL
    - Thrift
    - Json for C++
    - TALib
    - ChartDirector

## Deployment

The tradery components that need to be deployed are:

- tradery.exe (the tradery service executable)
- core.dll
- datasource.dll
- fileplugins.dll
- misc.dll
- miscwin.dll

These dlls are generated under the solution directory at: `bin/<debug|release>/<x86|x64>/`, depending on the type of target.

OpenSSL and ChartDirector dlls must also be in the PATH (names as of this writing):

- libcrypto-1_1.dll
- libssl-1_1.dll
- chartdir60.dll

## Configuration
These configuration files are currently located in the _tradery_ project:
- tradery.conf - common parameters
- tradery_debug.conf - parameters used when running the debug version
- tradery_release.conf - parameters used when running the release version

The format is:
```
# comment
name1=value
name2="value with spaces"
```

Check out src/tradery/tradery.conf file for more details on supported properties.

## Running tradery.exe
The executable tradery.exe accepts many command line arguments. Some are used to control startup mode: service or executable, normal or debug, 
while others are used to pass trading system parameters (similar to what would be passed in calls to the API) for one off tradery sessions, in which case 
the application starts, runs the session, and exits.

To run the tradery service as a regular executable from the command line, enter:
```tradery.exe -G```

As mentioned above, this project update is work in progress and currently only the 32 bit debug version runs properly. 
The release 32 bit version builds fine but still has some runtime issues, while the build update to the 64 bit version has not been completed yet.

## Charting

TE uses the ChartDirector package to generate charts (see _3rd Party Components_ section for details). 

TE generates the equity curve and associated statistics chart, price/volume charts for each symbol, as well
as user defined charts based on various indicators or other time series. For the latter, TSDL provides a number of primitives that allow users to define additional panes parallel to
the main symbol price pane, and what to display in these panes and how.

TE generates directly only the equity curve chart, which is a png file found in the session directory upon a successful run.

To generate the symbol price and additional user defined charts, the TE shares the work with the module that will actually display these charts. 

First, during a session, TE saves a number of files 
with all the information describing the charts, panes, values and other attributes as XML files under the _session/charts_ directory. 

After the session is complete, these files are used by the UI module to actually display the charts to the user.

The reason for this split is that ChartDirector has the ability to generate financial charts and manipulate them at viewing time based on user commands (zoom in/out, scroll etc), 
and after each command, a new chart is generated, correspnding to the area being displayed and its level of zoom.

## Logging

The configuration contains a parameter for specifying the name of the session log file. 

## Testing

To test the application, once the executable has been deployed and started (either as a service or a regular exe from the command line), it will be listening on the port set in the configuration 
file (or 9090 if not set) for thrift API calls. 

The solution comes with a test client (project _testthriftclient_), which will start a number of concurrent sessions (which can be configured in the code) and monitor their progress. The test trading systems, data and symbols are all 
part of the solution.

To test, build and run this project (once tradery.exe is running) and you will see the various session messages if everything is setup
and configured correctly. If not, various error messages should indicate the source of the problem(s).

## Security
Security is a very important aspect for this application, as it allows running arbitrary user code on the host system.

The first line of defense is to impose certain limitations on users' system scripts. For example, no preprocessor commands can be
used (#define, #include etc), there is no access to system functions, and other features of the C++ language are hidden as well.

However, the assumption is that an inventive hacker could and will come up with a way to circumvent these limiations.

The second line of defense is to ensure that if a weakness has been found, exploiting it won't give access to any critical system resources.

This is best done by running the executable or service on a dedicated server with no access to critical data, either locally or over the network, and/or from a 
user account with very limited permissions, just enough to allow it to read the data and generate the output files.


## License
Tradery Trading Engine is free open-source software distributed under the Terms and Conditions of the GNU General Public License (GPL) version 3 or (at your option) any later version.

For using Tradery Trading Engine, no restrictions apply. You can further redistribute and/or modify this software under the terms of the GPL.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

For details, have a look at the full text of the GPL in the LICENSE file or at http://www.gnu.org/licenses/

## 3rd party components
Tradery Trading Engine uses several 3rd party components:

- **boost**, a set of portable C++ libraries: http://www.boost.org
- **JSON for modern C++**, a JSON C++ library: https://github.com/nlohmann/json
- **OpenSSL**, a TLS and SSL toolkit: https://www.openssl.org
- **Apache Thrift**, a cross language service development framework: https://thrift.apache.org/
- **Chart Director**, a charting library: https://www.advsofteng.com. This is a commercial package that requires a license for production use. It can also be evaluated for free,
 in which case it will create charts with a footer text indicating that they were generated with an unlicensed version of the software. To set a license, look for the line "Chart::setLicenseCode" in the solution, copy the code as indicated, 
and uncomment the line.

Also, the following applications have been used during development:

* **Visual Studio 2015 Community Edition**: https://www.visualstudio.com/vs/older-downloads/ with **Service Update 3**: https://www.visualstudio.com/en-us/news/releasenotes/vs2015-update3-vs
* **Git Command Line Client for Windows and Linux**: https://git-scm.com/downloads.
* **clang-format**, a command line tool used to format C and C++ source code: http://releases.llvm.org/5.0.0/tools/clang/docs/ClangFormat.html. Used
the linux version - install on Ubuntu with ```sudo apt install clang-format-5.0```
* **VirtualBox**: https://www.virtualbox.org, with Windows as host and running Ubuntu as guest OS, used for general file and git operations.

