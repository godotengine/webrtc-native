
def exists(env):
    return True


def generate(env):
    env.AddMethod(cmake_configure, "CMakeConfigure")
    env.AddMethod(cmake_build, "CMakeBuild")


def cmake_configure(env, source, target, opt_args):
    args = [
        "-B",
        target,
    ]
    if env["platform"] == "windows" and env["use_mingw"]:
        args.extend(["-G", "Unix Makefiles"])
    for arg in opt_args:
        args.append(arg)
    args.append(source)
    return env.Execute("cmake " + " ".join(['"%s"' % a for a in args]))


def cmake_build(env, source, target=""):
    jobs = env.GetOption("num_jobs")
    env = env.Clone()
    return env.Execute("cmake --build %s %s -j%s" % (source, "-t %s" % target if target else "", jobs))
