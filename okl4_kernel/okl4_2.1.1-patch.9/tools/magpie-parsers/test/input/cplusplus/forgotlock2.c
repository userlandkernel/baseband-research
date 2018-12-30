void
entryfunc(int param1)
{
	lock();

	if (param1 == 0) {
		unlock();
	}
}
