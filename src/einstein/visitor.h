#ifndef __VISITOR_H__
#define __VISITOR_H__


/// Abstract visitor
template <typename T>
class Visitor
{
    public:
        virtual ~Visitor() { };

        /// Called at every visit
        virtual void onVisit(T &t) = 0;
};


#endif

