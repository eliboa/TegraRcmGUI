#pragma once

template<typename Func>
class ScopeGuard
{
public:
	ScopeGuard(Func&& runFunc) : exitFunc(std::forward<Func>(runFunc)), shouldRun(true) {}
	~ScopeGuard()
	{
		if (shouldRun)
			exitFunc();
	}

	bool reset() { auto prevRun = shouldRun; shouldRun = false; return prevRun; }
	bool run() { auto prevRun = shouldRun; if (shouldRun) { exitFunc(); shouldRun = false; } return prevRun; }
private:
	Func exitFunc;
	bool shouldRun;
};

template<typename Func>
ScopeGuard<Func> MakeScopeGuard(Func&& theFunc) { return { std::forward<Func>(theFunc) }; }