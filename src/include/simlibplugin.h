/*
Copyright (C) 2018 Adrian Michel
http://www.amichel.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "plugin.h"
#include "datasource.h"
#include "core.h"

namespace tradery {

/**
 * \defgroup PluginTypes Plugin types
 *
 * The plugin types currently supported
 * @{
 */

/**
 * DataSource plugin
 */
typedef Plugin<DataSource> DataSourcePlugin;
/**
 * SymbolsSource plugin
 */
typedef Plugin<SymbolsSource> SymbolsSourcePlugin;
/**
 * Runnable plugin
 */
typedef Plugin<Runnable> RunnablePlugin;
/**
 * Slippage plugin
 */
typedef Plugin<Slippage> SlippagePlugin;
/**
 * Commission plugin
 */
typedef Plugin<Commission> CommissionPlugin;
/**
 * Signal handler plugin
 */
typedef Plugin<SignalHandler> SignalHandlerPlugin;

// end plugin types
/*@}*/

}  // namespace tradery

extern "C" {

/**
 * \defgroup PluginNonMember Non member plugin APIs
 * '
 * For a plugin to be discovered at runtime, it must implement one of the get...
 * methods
 *
 * The release... methods are optional, and can be used for resource cleanup
 * when the plugin is to be released
 */
/*@{*/
PLUGIN_API tradery::DataSourcePlugin* getDataSourcePlugin();
PLUGIN_API tradery::SymbolsSourcePlugin* getSymbolsSourcePlugin();
PLUGIN_API tradery::RunnablePlugin* getRunnablePlugin();
PLUGIN_API tradery::SlippagePlugin* getSlippagePlugin();
PLUGIN_API tradery::CommissionPlugin* getCommissionPlugin();
PLUGIN_API tradery::SignalHandlerPlugin* getSignalHandlerPlugin();

PLUGIN_API void releaseDataSourcePlugin();
PLUGIN_API void releaseSymbolsSourcePlugin();
PLUGIN_API void releaseRunnablePlugin();
PLUGIN_API void releaseSlippagePlugin();
PLUGIN_API void releaseCommissionPlugin();
PLUGIN_API void releaseSignalHandlerPlugin();
/*@}*/
}
