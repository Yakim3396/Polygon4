/*
 * Polygon-4
 * Copyright (C) 2015 lzwdgc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

using System;
using System.IO;
using System.Diagnostics;

using UnrealBuildTool;

public class Polygon4 : ModuleRules
{
    private string ModulePath
    {
        get { return Path.GetDirectoryName(RulesCompiler.GetModuleFilename(this.GetType().Name)); }
        //get { return Path.GetDirectoryName(RulesCompiler.GetFileNameFromType(this.GetType())); }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
    }

    private static int NumberOfCalls = 0;

	public Polygon4(TargetInfo Target)
	{
		PublicDependencyModuleNames .AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Landscape" });
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        LoadCoreModule(Target, "Engine");
	}

    void MakeDir(string dst)
    {
        try
        {
            System.IO.Directory.CreateDirectory(Path.GetDirectoryName(dst));
        }
        catch (System.Exception)
        {
        }
    }

    bool CopyLibrary(string src, string dst, bool overwrite)
    {
        bool copied = true;
        string[] ext = { ".dll", ".pdb" };
        MakeDir(dst);
        foreach (var e in ext)
        {
            //System.Console.WriteLine("Copying " + (src + e));
            try
            {
                File.Copy(src + e, dst + e, overwrite);
            }
            catch (System.Exception)
            {
                System.Console.WriteLine("Cannot copy " + (src + e));
                copied = false;
            }
        }
        return copied;
    }

    bool RemoveLibrary(string dst)
    {
        bool removed = true;
        string dll = dst + ".dll";
        string pdb = dst + ".pdb";
        if (File.Exists(dll))
        {
            try
            {
                string msg = "Deleting old hot reload file: \"{0}\".";
                File.Delete(dll);
                System.Console.WriteLine(msg, dll);
                File.Delete(pdb);
                System.Console.WriteLine(msg, pdb);
            }
            catch (System.Exception)
            {
                removed = false;
            }
        }
        return removed;
    }

    bool RemoveLogs(string dst)
    {
        bool removed = true;
        string log_debug = dst + ".log.debug";
        string log_trace = dst + ".log.trace";
        try
        {
            File.Delete(log_debug);
        }
        catch (System.Exception)
        {
            removed = false;
        }
        try
        {
            File.Delete(log_trace);
        }
        catch (System.Exception)
        {
            removed = false;
        }
        return removed;
    }
    
    public void LoadCoreModule(TargetInfo Target, string Name)
    {
        //System.Console.WriteLine("Enter LoadCoreModule");

        if (Target.Platform != UnrealTargetPlatform.Win64 && Target.Platform != UnrealTargetPlatform.Win32)
            return;

        PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, Name, "include"));
        PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "DataManager/include"));
        PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "../../DataManager/include"));

        string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "x64" : "x86";
        string BaseDir = Path.Combine(ThirdPartyPath, Name, "lib");
        BaseDir = Path.GetFullPath(BaseDir);

        string base_name = Name + "." + PlatformString;
        int base_name_id = 0;
        
        PublicAdditionalLibraries.Add(BaseDir + "/" + base_name + ".lib");
        PublicDelayLoadDLLs.Add(base_name + ".dll");

        string dst_base_name = Path.Combine(ModulePath, "../../Binaries/", Target.Platform.ToString()) + "/" + base_name;
        dst_base_name = Path.GetFullPath(dst_base_name);

        string src = Path.Combine(BaseDir, base_name);
        string dst = dst_base_name;

        //if (Target.Type != TargetRules.TargetType.Editor)
        //    return;
        
        if (NumberOfCalls++ > 0)
            return;
        
        bool copied = CopyLibrary(src, dst, true);

        if (copied)
        {
            // try to remove previous dll, pdb
            base_name_id = 1;
            while (true)
            {
                dst = dst_base_name;
                if (base_name_id > 0)
                    dst = dst_base_name + "." + base_name_id.ToString();
                if (!RemoveLibrary(dst))
                    break;
                if (base_name_id++ > 1000)
                    break;
            }
            // try to remove previous logs
            base_name_id = 0;
            while (true)
            {
                dst = dst_base_name;
                if (base_name_id > 0)
                    dst = dst_base_name + "." + base_name_id.ToString();
                if (!RemoveLogs(dst))
                    break;
                if (base_name_id++ > 1000)
                    break;
            }
        }
    }
}
