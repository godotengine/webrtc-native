def exists(env):
    return True


def generate(env):
    env["DEPS_SOURCE"] = env.Dir("#thirdparty").abspath
    env["DEPS_BUILD"] = env.Dir("#bin/thirdparty").abspath + "/{}.{}.dir".format(
        env["suffix"][1:], "RelWithDebInfo" if env["debug_symbols"] else "Release"
    )
