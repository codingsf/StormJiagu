.class public Lcom/storm/tools/Native;
.super Ljava/lang/Object;
.source "Native.java"


# direct methods
.method static constructor <clinit>()V
    .locals 1

    .prologue
    .line 8
    const-string v0, "dexload"

    invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V

    .line 9
    return-void
.end method

.method public constructor <init>()V
    .locals 0

    .prologue
    .line 12
    invoke-direct {p0}, Ljava/lang/Object;-><init>()V

    .line 13
    return-void
.end method

.method public static native attachBaseContext(Landroid/content/Context;)V
.end method

.method public static native loadPluginDex(Ljava/lang/String;)Z
.end method

.method public static native loadXposedDex(Ljava/lang/String;Ljava/lang/Object;)Z
.end method
