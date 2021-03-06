﻿// LASP - Low-latency Audio Signal Processing plugin for Unity
// https://github.com/keijiro/Lasp

using System;
using System.Runtime.InteropServices;

namespace Lasp
{
    public enum FilterType { Bypass, LowPass, BandPass, HighPass }

    public static class PluginEntry
    {
        #region Plugin interface

        #if UNITY_IOS
        const string _libName = "__Internal";
        #else
        const string _libName = "Lasp";
        #endif

        [DllImport(_libName, EntryPoint="LaspCreateDriver")]
        public static extern IntPtr CreateDriver();

        [DllImport(_libName, EntryPoint="LaspDeleteDriver")]
        public static extern void DeleteDriver(IntPtr driver);

        [DllImport(_libName, EntryPoint="LaspOpenStream")]
        public static extern bool OpenStream(IntPtr driver);

        [DllImport(_libName, EntryPoint="LaspCloseStream")]
        public static extern void CloseStream(IntPtr driver);

        [DllImport(_libName, EntryPoint="LaspGetSampleRate")]
        public static extern float GetSampleRate(IntPtr driver);

        [DllImport(_libName, EntryPoint="LaspGetPeakLevel")]
        public static extern float GetPeakLevel(IntPtr driver, FilterType filter, float duration);

        [DllImport(_libName, EntryPoint="LaspCalculateRMS")]
        public static extern float CalculateRMS(IntPtr driver, FilterType filter, float duration);

        [DllImport(_libName, EntryPoint="LaspRetrieveWaveform")]
        public static extern int RetrieveWaveform(IntPtr driver, FilterType filter, float[] dest, int length);

        #endregion

        #region Debug helpers

        public static void SetupLogger()
        {
            #if !UNITY_IOS // FIXME: cant marchal the callback like this on iOS
            var del = (PrintDelegate)Log;
            var ptr = Marshal.GetFunctionPointerForDelegate(del);
            ReplaceLogger(ptr);
            #endif
        }

        [DllImport(_libName, EntryPoint="LaspReplaceLogger")]
        public static extern void ReplaceLogger(IntPtr logger);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void PrintDelegate(string message);

        static void Log(string message)
        {
        #if UNITY_EDITOR
            UnityEngine.Debug.Log(message);
        #else
            System.Console.WriteLine(message);
        #endif
        }

        #endregion
    }
}
