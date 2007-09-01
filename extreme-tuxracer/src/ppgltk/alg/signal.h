/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Peter Reichel <peter@apps4linux.org>
 *                         Volker Stroebel <volker@planetpenguin.de>
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#ifndef PP_SIGNAL_H
#define PP_SIGNAL_H

#include <stdlib.h>

namespace pp{


class BaseSlot;
class BaseSignal;
class BaseArgument;

class BaseArgument {
public:
	BaseArgument(int cnt) : mCnt(cnt){};

	virtual ~BaseArgument() {}
	int Count() const { return mCnt; }
private:
	int mCnt;
};

class Argument0 : public BaseArgument {
public:
	Argument0() : BaseArgument(0) { }
	~Argument0() { }
};

template<class R1>
class Argument1 : public BaseArgument {
public:
	Argument1(R1 a1) : BaseArgument(1)
	{
		mArg1 = a1;
	}
	~Argument1() { }

	R1 Arg1() const { return mArg1; }
private:
	R1 mArg1;
};

template<class R1, class R2>
class Argument2 : public BaseArgument {
public:
	Argument2(R1 a1, R2 a2) : BaseArgument(2)
	{
		mArg1 = a1;
		mArg2 = a2;
	}
	~Argument2() { }

	R1 Arg1() const { return mArg1; }
	R2 Arg2() const { return mArg2; }
private:
	R1 mArg1;
	R2 mArg2;
};

//Slotklasse

class BaseSlot {
public:
	BaseSlot() { }
	virtual ~BaseSlot() { }

	virtual void Call(BaseArgument *arg) = 0;
};

template<class T>
class Slot0 : public BaseSlot {
public:
	typedef void (T::*SlotPtr_t)();
	Slot0(T *rcv, SlotPtr_t sl)
	{
		mRcv = rcv;
		mSlot = sl;
	}
	~Slot0() { }

	void Call(BaseArgument *arg)
	{
		
		//Argumente brauchen ned ausgewertet werden => 0
		return (*mRcv.*mSlot)();
	}
private:
	T *mRcv;
	SlotPtr_t mSlot;
};


template<class T, class R1>
class Slot1 : public BaseSlot {
public:
	typedef void (T::*SlotPtr_t)(R1);
	Slot1(T *rcv, SlotPtr_t sl)
	{
		mRcv = rcv;
		mSlot = sl;
	}
	~Slot1() { }

	void Call(BaseArgument *arg)
	{
		if (arg->Count() == 1) {
			Argument1<R1> *a1 = static_cast<Argument1<R1>* >(arg);
			if (arg) (*mRcv.*mSlot)(a1->Arg1());
		}

		//mit RTTI
//		Argument1<R1> *a1 = dynamic_cast<Argument1<R1>* >(arg);
//		if (arg) (*mRcv.*mSlot)(a1->Arg1());
	}
private:
	T *mRcv;
	SlotPtr_t mSlot;
};

template<class T, class R1, class R2>
class Slot2 : public BaseSlot {
public:
	typedef void (T::*SlotPtr_t)(R1,R2);
	Slot2(T *rcv, SlotPtr_t sl)
	{
		mRcv = rcv;
		mSlot = sl;
	}
	~Slot2() { }

	void Call(BaseArgument *arg)
	{
		if (arg->Count() == 2) {
			Argument2<R1,R2> *a1 = static_cast<Argument2<R1,R2>* >(arg);
			if (arg) (*mRcv.*mSlot)(a1->Arg1(),a1->Arg2());
		}
	}
private:
	T *mRcv;
	SlotPtr_t mSlot;
};

//Signalklasse

class BaseSignal {
public:
	BaseSignal() : mSlot(NULL)
	{
	}
	
	virtual ~BaseSignal()
	{
		Clean();
	}
	
	void Connect(BaseSlot *slot)
	{
		Clean();
		mSlot = slot;
	}
private:
	//only to ensure noone tries to do this
	BaseSignal(const BaseSignal&);	
	BaseSignal operator=(const BaseSignal&);

	void Clean()
	{
		if (mSlot) {
			delete mSlot;
			mSlot = NULL;
		}
	}
protected:
	BaseSlot *mSlot;
};

class Signal0 : public BaseSignal {
public:
	Signal0() { }
	~Signal0() { }

	void Emit()
	{
		if (mSlot) {
			Argument0 arg;
			mSlot->Call(&arg);
		}
	}
};

template<class R1>
class Signal1 : public BaseSignal {
public:
	Signal1() { }
	~Signal1() { }

	void Emit(R1 r1)
	{
		if (mSlot) {
			Argument1<R1> arg(r1);
			mSlot->Call(&arg);
		}
	}
};

template<class R1, class R2>
class Signal2 : public BaseSignal {
public:
	Signal2() { }
	~Signal2() { }

	void Emit(R1 r1, R2 r2)
	{

		if (mSlot) {
			Argument2<R1,R2> arg(r1,r2);
			mSlot->Call(&arg);
		}
	}
};

//Creation function

template<class T>
BaseSlot* CreateSlot(T* rcv, void (T::*sl)())
{
	return new Slot0<T>(rcv,sl);
}

template<class T, class R1>
BaseSlot* CreateSlot(T* rcv, void (T::*sl)(R1))
{
	return new Slot1<T,R1>(rcv,sl);
}

template<class T, class R1, class R2>
BaseSlot* CreateSlot(T* rcv, void (T::*sl)(R1,R2))
{
	return new Slot2<T,R1,R2>(rcv,sl);
}

}// namepsace pp

#endif // PP_SIGNAL_H
