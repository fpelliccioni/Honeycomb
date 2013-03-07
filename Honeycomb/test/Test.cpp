// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Test.h"
#include "Honey/Honeycomb.h"

namespace honey
{

void test()
{
    //=============================
    //LockFree list test
    //=============================
    {
        typedef lockfree::List<int> List;

        struct ListThread
        {
            static void run(List& list)
            {
                Salsa rand;
                int data;
                int count = 500;
                for (int i = 0; i < count; ++i)
                {
                    switch (Discrete(rand, 0, 5).nextInt())
                    {
                    case 0:
                        list.pushFront(i);
                        break;
                    case 1:
                        list.pushBack(i);
                        break;
                    case 2:
                        list.popFront(data);
                        break;
                    case 3:
                        list.popBack(data);
                        break;
                    case 4:
                        {
                            auto it = list.begin();
                            for (int j = 0, end = Discrete(rand, 0, list.size()).nextInt(); j < end; ++j) ++it;
                            list.insert(it, i);
                            break;
                        }
                    case 5:
                        {
                            auto it = list.begin();
                            for (int j = 0, end = Discrete(rand, 0, list.size()).nextInt(); j < end; ++j) ++it;
                            if (it != list.end()) list.erase(it, data);
                            break;
                        }
                    }
                }
            }            
        };
        
        List list;
        vector<Thread> threads;
        for (auto i : range(7)) { threads.push_back(Thread([&]{ ListThread::run(list); })); mt_unused(i); }
        for (auto& e : threads) e.start();
        for (auto& e : threads) e.join();

        int count = 0;
        debug::print(sout() << "List Size: " << list.size() << endl);
        for (auto& e : list) debug::print(sout() << "List " << count++ << " : " << e << endl);
    }
    //=============================

    //=============================
    //Concurrent deque test
    //=============================
    {
        typedef concur::Deque<int> List;

        struct ListThread
        {
            static void run(List& list)
            {
                Salsa rand;
                int data;
                int count = 500;
                for (int i = 0; i < count; ++i)
                {
                    switch (Discrete(rand, 0, 3).nextInt())
                    {
                    case 0:
                        list.pushFront(i);
                        break;
                    case 1:
                        list.pushBack(i);
                        break;
                    case 2:
                        list.popFront(data);
                        break;
                    case 3:
                        list.popBack(data);
                        break;
                    }
                }
            }
        };
        
        List list;
        vector<Thread> threads;
        for (auto i : range(10)) { threads.push_back(Thread([&]{ ListThread::run(list); })); mt_unused(i); }
        for (auto& e : threads) e.start();
        for (auto& e : threads) e.join();

        int data;
        int count = 0;
        debug::print(sout() << "Deque Size: " << list.size() << endl);
        while (list.popFront(data))
        {
            debug::print(sout() << "Deque " << count << " : " << data << endl);
            ++count;
        }          
    }
    //=============================
    
    task::priv::test();

    {
        int aa = 1;
        Promise<int> promise;
        promise.setValue(aa);
        Future<int> future = promise.future();
        auto blah = future.wait(Seconds(1));    mt_unused(blah);
        
        Promise<int> promise2;
        aa = 2;
        promise2.setValue(aa);
        Future<int> future2 = promise2.future();

        future::waitAll(future, future2);
        int i = future::waitAny(future, future2);   mt_unused(i);

        vector<Future<int>> futures;
        futures.push_back(move(future));
        futures.push_back(move(future2));
        future::waitAll(futures);
        vector<Future<int>>::iterator it = future::waitAny(futures);    mt_unused(it);
        
        SharedFuture<int> shared = futures[0].share();
        SharedFuture<int> shared2 = shared;
        int bb = shared.get();
        bb = shared2.get();
        bb = futures[1].get();

        PackagedTask<void (int)> task = [](int a) { !a ? throw_ Exception() << "test0" : throw std::runtime_error("test1"); };
        for (auto i : range(2))
        {
            task(i);
            try { task.future().get(); } catch (Exception& e) { debug::print(e.what_()); }
            task.reset();
        }

        PackagedTask<int& (int&)> task2([](int& a) -> int& { return a; }, std::allocator<int>());
        task2(aa);
        ++task2.future().get();
    }

    {
        int a = 0;
        for (auto i : range(0, 5, 2)) { mt_unused(i); ++a; }
        for (auto i : range(0.0, 4.0, 1.3)) { mt_unused(i); ++a; }
        for (auto i : range(2)) { mt_unused(i); ++a; }
        for (auto i : range(4, 0, -2)) { mt_unused(i); ++a; }
        for (auto i : range(4.0, 0.0, -1.3)) { mt_unused(i); ++a; }
    }

    {
        int i = 0;
        {
            auto _ = ScopeGuard([&] { i = 1; });
            auto __ = ScopeGuard([&] { i = 2; });
            _.release();
            i = 3;
        }
    }

    {
        int count = 1000;
        BloomFilter<int> bloom(count, 0.01);
        for (auto i : range(count)) { bloom.insert(i); }
        
        int error = 0;
        for (auto i : range(count)) { if (!bloom.contains(i)) ++error; }
        // error == 0
        for (auto i : range(count)) { if (bloom.contains(count+i)) ++error; }
        // error ~= count * 0.01
    }

    {
        Real sinDif = -Real_::inf;
        int size = 1000;
        for (auto i : range(size))
        {
            Real angle = -Real_::piTwo*2 + i*Real_::piTwo*4 / (size-1);
            Real sin = Trig::atan2(Trig::cos(angle)*2, Trig::sin(angle)*2);
            Trig::enableSinTable(true);
            Real sinTab = Trig::atan2(Trig::cos(angle)*2, Trig::sin(angle)*2);
            Trig::enableSinTable(false);
            sinDif = Alge::max(sinDif, Alge::abs(sinTab - sin));
            //debug::print(sout() << "Sin: " << sin << endl << "Tab: " << sinTab << endl << endl);
        }

        debug::print(sout() << "Sin Table Dif: " << sinDif << endl);
    }

    {
        vector<Vec3> vecs;
        vecs.push_back(Vec3(0,1,2));
        vecs.push_back(Vec3(3,4,5));
        vecs.push_back(Vec3(6,7,8));
        Vec3 blended = Interp::blend(vecs, Vec3(1,4,2));    mt_unused(blended);
        
        VecN v(20);
        v[10] = 1;
        v.resize(4);
        v(2) = 1;
        v = Vec3(1, 2, 3);
        v = v;
        v = VecN(3).fromZero();
        Vec3 v3 = v;
        v3.normalize();
        VecRowN vr(3);
        vr[2] = 0;
        vr(0,1) = 1;
        vr = Vec1(1);

        Double arr[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
        auto mata = Matrix4().fromArray(arr);
        mata = Matrix4().fromArray(arr, false);
        mata.toArray(arr);
        mata.toArray(arr, false);

        auto v41 = Matrix<4, 1>(2) * 2;     mt_unused(v41);
        
        Real sum = Matrix4().fromScalar(1).elemAdd(1).sum(); mt_unused(sum);
        Vec2 swiz2(1,2);
        Vec3 swiz3(1,2,3);
        Vec4 swiz4(1,2,3,4);
        swiz2.yx() += 1;
        map(swiz4, swiz4.wzxy(), [](Real e) { return e+1; });
        swiz4.wzyx() = 2 * swiz4.xyzw();
        swiz3.yzx() = swiz3.xzz() + swiz3.xyz();
        swiz3 = swiz4.elemMul(swiz2.xxxy()).xzz();

        auto mat3 = Vec3(1,2,3) * VecRow3(10,100,1000);
        auto mat8 = (Matrix<8,8>().fromIdentity().block<4,4>(3,2).block<2,2>(1,2) = (Matrix<2,2>() << 2,3,4,5).eval()).parent().parent();

        debug::print(sout() << "Mat3: " << endl << mat3 << endl
                            << "Mat8: " << endl << mat8 << endl
                            << "Sum8, Min8: " << mat8.sum() << ", " << mat8.min() << endl);
    }

    {
        //Test data from wikipedia "QR decomposition"
        //Q(0,:) = (-0.86,  0.39, -0.33)
        //R(0,:) = (-14, -21, 14)
        auto A = (Matrix<3,3>() <<
            12, -51, 4,
            6, 167, -68,
            -4, 24, -41).eval();
        Qrd<Real> qrd(A);
        debug::print(sout() << "Q: " << endl << qrd.q() << endl
                            << "R: " << endl << qrd.r() << endl
                            << "A = Q*R: " << endl << qrd.q()*qrd.r() << endl);
    }

    {
        //Test data from wikipedia "Jacobi eigenvalue algorithm"
        //Eigen Val: (2585.25, 37.1015, 1.47805, 0.166633)
        auto A = (Matrix<4,4>() <<
            4,  -30,    60,     -35,
            -30, 300,   -675,   420,
            60, -675,   1620,   -1050,
            -35, 420,   -1050,  700).eval();
        MatrixN inv;
        auto eigen = Eigen<Real>(A);
        eigen.inverse(inv);
        debug::print(sout() << "Eigen Val: " << eigen.w() << endl
                            << "Eigen Vec: " << endl << eigen.v() << endl
                            << "I = A*EigenInv(A): " << endl << A*inv << endl);
    }

    {
        //Linear LeastSqr b: (3.02448, 1.12965) ; cond: 14.994
        auto X = (Matrix<5,2>() <<
            1, 0,
            1, 3,
            1, 7,
            1, 10,
            1, 16).eval();
        auto y = (Vec<5>() << 3, 5, 10, 17, 20).eval();
        auto w = (Vec<5>() << 2, 3, 1, 4, 5).eval();
        VecN b;
        LinearLeastSqr<Real>().calc(X, y, w, b);
        debug::print(sout() << "Linear LeastSqr b: " << b << " ; cond: " << X.cond() << endl);

        Svd<Real> svd(X, Svd<Real>::Mode::full);
        MatrixN inv;
        svd.inverse(inv);
        debug::print(sout() << "X*FullSvdInv(X)*X: " << endl << X*inv*X << endl);
    }

    {
        //Test data from "Numerical Methods for Least Squares Problems"
        //b: (5.75, -0.25, 1.5) ; cond: 2
        auto X = (Matrix<4,3>() <<
            1, 1, 1,
            1, 3, 1,
            1, -1, 1,
            1, 1, 1).eval();
        auto y = (Vec<4>() << 1, 2, 3, 4).eval();
        auto w = (Vec<4>() << 1, 1, 1, 1).eval();
        auto C = (Matrix<2,3>() <<
            1, 1, 1,
            1, 1, -1).eval();
        auto d = (Vec<2>() << 7, 4).eval();
        VecN b;
        LinearLeastSqr<Real>().calc(X, y, w, C, d, b);
        debug::print(sout() << "Constrained LeastSqr b: " << b << " ; cond: " << X.cond() << endl);        
    }

    {
        //BisectN: true; (3.14159, 1.5708, 1.5708)
        typedef BisectN<Real,3> BisectN;
        BisectN::Funcs funcs =
        {
            [](const Vec3& v) { return v.x - v.y - Real_::piHalf; },
            [](const Vec3& v) { return v.x - v.y - v.z; },
            [](const Vec3& v) { return v.z - v.x + Real_::piHalf; },
        };
        auto res = BisectN().root(funcs, Vec3(-10), Vec3(10));
        debug::print(sout() << "BisectN: " << std::boolalpha << get<0>(res) << "; " << get<1>(res) << endl);
    }

    {
        //Mean CI 95%:    (-9.19019, -2.52411)
        //Std Dev CI 95%: (2.05223, 4.72423)
        //Pr > |t|:       0.00240068
        auto vf = (Vec<7>() << 75, 76, 80, 77, 80, 77, 73).eval();
        auto vm = (Vec<7>() << 82, 80, 85, 85, 78, 87, 82).eval();
        StudentT::PooledStats stats;
        bool t_test = StudentT::test(vf, vm, stats, 0, 0.05, 0);
        debug::print(sout() << "T-test: " << std::boolalpha << t_test << endl << stats << endl);
    }

    //Minimize: (2.23277, -1.99996)
    debug::print(sout() << "Minimize: " <<
                    Minimize<Real>().calc([](Real x) { return 0.2f*Alge::pow(x,4) - 2*Alge::pow(x,2) + 3; }, 0, 5, 0.1) << endl);

    //MinimizeN: ((1, 2), 0)
    debug::print(sout() << "MinimizeN: " <<
                    MinimizeN<Real,2>().calc([](const Vec2& v) { return Alge::pow(v[0]-1, 2) + Alge::pow(v[1]-2, 2); }, Vec2(-10), Vec2(10), Vec2(7, -5)) << endl);

    //Poly roots 3: ((-0.60583, 0, 0), 1) Bounds: (0.2, 1.75)
    //Poly roots 4: ((5, 3, -4, -6), 4)
    debug::print(sout() << "Poly roots 1: " << Polynomial<Real>::roots(Vec2(1,2)) << endl);
    debug::print(sout() << "Poly roots 2: " << Polynomial<Real>::roots(Vec3(1,2,3)) << endl);
    debug::print(sout() << "Poly roots 3: " << Polynomial<Real>::roots(Vec4(1,2,3,4)) << " Bounds: " << Polynomial<Real>::rootBounds(Vec4(1,2,3,4)) << endl);
    debug::print(sout() << "Poly roots 4: " << Polynomial<Real>::roots((Vec<5>() << 1080,-126,-123,6,3).eval()) << endl);
    debug::print(sout() << "Poly roots 4 (generic): " << Polynomial<Real>::roots((VecN().resize(5) << 1080,-126,-123,6,3).eval(), 1e-04f) << endl);

    assert(String::join(String("foo bar blah").split()) == "foo bar blah");
    assert(String::join(String::List() << "foo" << "bar" << "blah") == "foo bar blah");

    Real f = 0.5, f2;
    Vec3 v1, v2, v3;
    v1 = Vec3(3.f, 4.5f, 1.2f);
    v1 = v1.normalize();

    v2 = Vec3(1.f, 2.5f, 3.2f);
    v2 = v2.normalize();

    v3 = Vec3(4.f, 0.5f, 2.2f);

    Quat q0, q1, q2, q3, q4;
    q0.fromAlign(v1, v2);
    q1.fromAlign(v2, v1);
    q2.fromEulerAngles(Vec3(4.f, 1.5f, 5.2f));
    q3.fromAxisAngle(Vec3::axisX, Real_::piHalf);
    
    Quat a, b, c;
    Quat::squadSetup(q0, q1, q2, q3, a, b, c);
    q4 = q2;
    q4 = q4.inverse();

    Transform tm, tm2, tm3, tm4;
    tm2.setTrans(v1);
    tm2.setScale(Vec3(1,2,2));
    tm3.setRot(q3);
    tm4.setScale(2);
    tm = tm2*tm3;
    tm = tm.inverse();
    tm = tm*tm4;

    Matrix4 mat, mat2, mat3, mat4;
    mat2.fromIdentity().setTrans(v1);
    mat2.setScale(Vec3(1,2,2));
    mat3.fromIdentity().setRot(q3);
    mat4.fromIdentity().setScale(2);
    mat = mat2*mat3;
    mat = mat.inverse();
    mat = mat*mat4;

    Line line;
    line.v0 = v1;
    line[1] = v2;
    line = mat * line;
    line = tm * line;

    Ray ray(v1, (v2-v1).normalize());
    ray = mat * ray;
    ray = tm * ray;

    Plane plane(ray.dir, v2);
    plane = mat * plane;
    plane = tm * plane;

    Triangle tri;
    tri.v0 = v1;
    tri[1] = v2;
    tri.v2 = v3;

    Sphere sphere;
    sphere.center = v1;
    sphere.radius = (v2-v1).length();

    Box box;
    box.min = v1;
    box.max = v2;

    OrientBox obox;
    obox.center = v1;
    obox.axis[0] = v1; obox.axis[1] = v1.crossUnit(Vec3::axisY); obox.axis[2] = v1.crossUnit(obox.axis[1]);
    obox.extent = v2;

    Cylinder cyl;
    cyl.center = v1;
    cyl.axis = v2;
    cyl.radius = f;
    cyl.height = 10;

    Cone cone;
    cone.vertex = v1;
    cone.axis = v2;
    cone.angle = f;
    cone.height = 10;

    Capsule cap;
    cap.line.v0 = v1;
    cap.line.v1 = v2;
    cap.radius = f;

    FrustumPersp persp(60, 0.5, 1, 100);
    FrustumOrtho ortho(-1, 1, -1, 1, 0, 1);

    tm.fromMatrix(mat);

    Box box1;
    box1.fromCenter(Vec3(6,0,0), Vec3(2, 1, 1));
    Ray ray1(Vec3(0,0,0), Vec3(1,0,0));
    Line line1(Vec3(1,0,0), Vec3(2,1,0));
    Triangle_f tri1(Vec3(2,-1,-2), Vec3(2,-1,2), Vec3(2,1,0));
    Real dist = Intersect::distance(box1, line1, v1, v2); mt_unused(dist);

    bool test2 = Intersect::find(tri1, line1, v1); mt_unused(test2);
    bool ba = Intersect::test(obox, obox); mt_unused(ba);

    typedef Vec<6> Vec6;
    auto v6 = Vec6().fromAxis(0);
    v6 += Vec6().fromScalar(1)*5;
    v6 = v6.normalize(f);

    typedef Matrix<6,6> Matrix6;
    auto mat6 = Matrix6().fromIdentity();
    mat6 += Matrix6(1)*5;
    mat6 = mat6.inverse(f);
    mat6.transposeInPlace();

    concur::Salsa gen;

    f = Uniform(gen).next();
    Double fd = Uniform_d(gen).next(); mt_unused(fd);
    f = Discrete(gen, 10, 20).next();
    f = Discrete(gen).next();
    f = Discrete(gen).pdf(0);
    f = Discrete(gen).cdf(100);
    f = Discrete(gen).cdfComp(100);
    f = Discrete(gen).cdfInv(0.5);
    f = Discrete(gen).mean();
    f = Discrete(gen).variance();
    f = Discrete(gen).stdDev();

    Salsa::State state = gen.getState();
    f2 = Gaussian(gen, 0, 10).next();

    DiscreteGen::List pdf;
    pdf.push_back(1);
    pdf.push_back(2);
    pdf.push_back(10);
    pdf.push_back(12);
    pdf.push_back(9);
    pdf.push_back(2);
    pdf.push_back(6);
    pdf.push_back(15);
    pdf.push_back(2);
    pdf.push_back(2);
    pdf.push_back(3);
    pdf.push_back(2);
    pdf.push_back(3);
    pdf.push_back(4);
    pdf.push_back(8);
    pdf.push_back(9);
    pdf.push_back(2);
    pdf.push_back(8);
    pdf.push_back(4);
    pdf.push_back(1);
    pdf.push_back(5);
    pdf.push_back(8);
    pdf.push_back(1);
    pdf.push_back(6);
    pdf.push_back(2);
    pdf.push_back(10);
    pdf.push_back(12);
    pdf.push_back(6);
    pdf.push_back(1);
    pdf.push_back(15);
    DiscreteGen disc(gen, pdf);
    //HyperGeo disc(gen, 200, 50, 90);
    debug::print(sout() << "Disc Mean: " <<  disc.mean() << " ; Disc Var: " << disc.variance() << endl);

    vector<Vec1> samples;
    for (auto i : range(100))
    {
        mt_unused(i);
        samples.push_back(disc.next());
    }

    typedef Bootstrap<Vec1> Bootstrap;

    Bootstrap bootMean(Bootstrap::meanFunc(), gen, samples);
    bootMean.calc();
    debug::print(sout() << "Boot Mean: " << bootMean.lower() << " ; " << bootMean.upper() << endl);

    Bootstrap bootVar(Bootstrap::varianceFunc(), gen, samples);
    bootVar.calc();
    debug::print(sout() << "Boot Var: " << bootVar.lower() << " ; " << bootVar.upper() << endl);

    for (auto i : range(-1, 50))
    {
        mt_unused(i);
        f = disc.next();
        f2 = disc.cdfInv(disc.cdf(f));
        debug::print(sout() << "Dif: " << std::setw(4) << f << " ; " << std::setw(4) << f2 << " ; " << Alge_d::abs(f-f2) << endl);
    }

    vector<Real> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    list.push_back(4);
    vector<Real> sample;
    vector<int> unchosen;
    Random(gen).choose(list, 3, sample, unchosen);
    Random(gen).shuffle(sample);

    struct PermuteFunc
    {
        bool operator()(const vector<const Real*>& perm)
        {
            if (perm.size() == 2 && *perm[0] == 1 && *perm[1] == 4)
                return false;
            if (perm.size() == 2 && *perm[0] == 2 && *perm[1] == 3)
                return false;
            if (perm.size() == 3 && *perm[0] == 2 && *perm[1] == 4 && *perm[2] == 3)
                return false;
            if (perm.size() == 3 && *perm[0] == 4 && *perm[1] == 3 && *perm[2] == 1)
                return false;
            return true;
        }
    };

    auto permute = Permute::range(list, PermuteFunc());
    for (auto it = begin(permute); it != end(permute); ++it)
    {
        debug::print("Perm: ");
        for (auto i : range(utos(it->size()))) { debug::print(sout() << *it->at(i) << " "); }
        debug::print(sout() << " ; Cnt: " << it.count() << "\n");
    }

    gen.setState(state);
    f2 = Gaussian(gen, 0, 10).next();

    Salsa crypt;
    Salsa::Key key;
    Salsa::Iv iv, iv2;
    for (auto i : range(key.size)) { key.a[i] = Discrete_<uint32>::nextStd(gen); }
    for (auto i : range(iv.size)) { iv.a[i] = Discrete_<uint32>::nextStd(gen); }
    for (auto i : range(iv.size)) { iv2.a[i] = Discrete_<uint32>::nextStd(gen); }
    
    String msg = "Test msg la la la la ";
    for (auto i : range(50)) { msg += sout() << i << " "; }
        
    String msg2 = "Test2 msg2 la la la la";
    uint8 cipher[1000];
    char decipher[1000];

    crypt.setKey(key);
    crypt.setIv(iv);
    crypt.encrypt(reinterpret_cast<const uint8*>(msg.u8().c_str()), reinterpret_cast<uint8*>(cipher), msg.length());
    crypt.setIv(iv2);
    crypt.encrypt(reinterpret_cast<const uint8*>(msg2.u8().c_str()), reinterpret_cast<uint8*>(cipher+msg.length()), msg2.length());

    crypt.setKey(key);
    crypt.setIv(iv);
    crypt.decrypt(reinterpret_cast<uint8*>(cipher), reinterpret_cast<uint8*>(decipher), msg.length());
    crypt.setIv(iv2);
    crypt.decrypt(reinterpret_cast<uint8*>(cipher+msg.length()), reinterpret_cast<uint8*>(decipher+msg.length()), msg2.length());
    decipher[msg.length()+msg2.length()] = 0;

    debug::print(sout() << "Hash  : " << Hash::toString(Hash::fast("Hash of some string")) << endl);
    debug::print(sout() << "Hash 2: " << Hash::toString(Hash::fast("Hash of some string2")) << endl);
    debug::print(sout() << "Secure Hash  : " << Hash::secure("Secure hash of some string") << endl);
    debug::print(sout() << "Secure Hash 2: " << Hash::secure("Secure hash of some string2") << endl);

    typedef Vegas<5, 4, Double> Vegas;
    struct VegasFunc
    {
        typedef Vegas::Real Real;
        typedef Alge_<Real> Alge;

        Vegas::VecRes operator()(const Vegas::Vec& x)
        {
            //Principal integral        0.999142 +/- 0.000812909
            //1st additional integral   2.71833  +/- 0.00406556
            //2nd additional integral   3.13637  +/- 0.00468428
            //3rd additional integral   0.998752 +/- 0.00115633
            Vegas::VecRes f;
            Real dummy, exponent, denominator;

            exponent = 0.0;
            denominator = 1.0;
            for (auto i : range(Vegas::dim))
            {
                dummy = 2*x[i]-1;
                dummy = 0.5*Alge::log((1+dummy)/(1-dummy));
                exponent -= dummy*dummy/2.0/0.2/0.2;
                dummy = 2*x[i]-1;
                denominator *= 1-dummy*dummy;
            }

            f[0] = Alge::exp(exponent)/denominator/Alge::pow(0.5*Real_::pi*0.2*0.2,Real(Vegas::dim)/2);
            if (Vegas::dimRes >= 2)
                f[1] = f[0] * 2.718281828 * (12.0*x[0]-5.0);
            if (Vegas::dimRes >= 3)
                f[2] = f[0] * 3.141592654 * (12.0*x[1]-5.0);
            if (Vegas::dimRes >= 4)
                f[3] = (x[2]<0.5) ? (f[0]*2.0) : (0.0);
            return f;
        }
    };

    Vegas vegas(VegasFunc(), gen, Vegas::Vec(0.), Vegas::Vec(1.), 10000);
    debug::print(sout() << "Vegas: " << vegas.integrate() << endl);

    Id id = ID"foo_bar";
    Id id2 = ID"foo_bar";
    assert(id == id2);

    #define IDSWITCH_LIST(c) c(foo_bar) c(eggs, "eggz")
    IDSWITCH(id)
    #undef IDSWITCH_LIST
    {
    IDCASE(eggs):
        {
            debug::print("IdSwitch: eggz\n");
            break;
        }
    IDCASE(foo_bar):
        {
            debug::print("IdSwitch: foo_bar\n");
            break;
        }
    IDCASE(default):
        {
            debug::print("IdSwitch: default\n");
            break;
        }
    }
    IDSWITCH_END

    typedef DepNode<int> DepNode;
    DepNode depnode[10];
    for (auto i : range(10)) { depnode[i].setKey(sout() << "Node " << i); }
        
    depnode[0].add("Node 1");
    depnode[0].add("Node 3");
    depnode[1].add("Node 2");
    depnode[2].add("Node 0");

    depnode[3].add("Node 4");
    depnode[4].add("Node 5");
    depnode[5].add("Node 3");

    depnode[6].setKey("Node 0");
    depnode[6].add("Node 1");
    depnode[6].add("Node 3");
    
    DepGraph<DepNode> depgraph;
    for (auto i : range(10)) { depgraph.add(depnode[i]); }
    
    bool depends = depgraph.depends(depnode[0].getKey(), depnode[5]);
    depends = depgraph.depends(depnode[5].getKey(), depnode[0].getKey());

    depgraph.condense();
    depgraph.remove(depnode[6]);
    depgraph.condense();

    int depvertex = 0;
    for (auto& e : depgraph.range(depnode[0].getKey()))
    {
        debug::print(sout() << "DepVertex " << depvertex++ << endl);
        for (auto& e : e.nodes()) { debug::print(sout() << "    " << e->getKey() << endl); }
    }

    SceneObject tree1, tree2;
    SceneObject obj1, obj2;
    tree1.setInstId("tree1");
    tree2.setInstId("tree2");
    obj1.setInstId("obj1");
    obj2.setInstId("obj2");
    tree1.com<Tm>().setInstId("test");
    tree1.com<Tm>("test");
    const vector<Tm::Ptr>& comlist = tree1.coms<Tm>();      mt_unused(comlist);

    obj1.addCom(new CullVol<Box>);
    obj1.com<CullVol<Box>>().setShape(Box().fromCenter(Vec3(0), Vec3(5)));

    obj2.addCom(new CullVol<Box>);
    obj2.com<CullVol<Box>>().setShape(Box().fromCenter(Vec3(11), Vec3(5)));

    CullBoxGen::Ptr boxgen = new CullBoxGen;
    obj1.addCom(boxgen);
    boxgen->update(true);
    obj1.removeCom(*boxgen);

    tree1.addCom(new OctTreeSpace(Box().fromCenter(Vec3(0), Vec3(100))), true);
    tree2.addCom(new OctTreeSpace(Box().fromCenter(Vec3(0), Vec3(100))), true);
    obj1.addCom(new TreeSpace);

    tree1.com<SceneSpace>().add(obj1);
    tree1.com<SceneSpace>().add(tree2);
    tree2.com<SceneSpace>().add(obj2);
    {
        auto range = tree1.com<Tree>().children("obj1");
        for (auto& e : stdutil::values(range)) { debug::print(sout() << "Match: " << e->getKey() << endl); }
    }

    struct EnumVisitor : public SceneSpace::EnumVisitor
    {
        virtual void operator()(const SceneSpace& space, SceneObject& obj)
        {
            mt_unused(space);
            debug::print(sout() << "Visit: " << obj.getInstId() << endl);
            if (obj.getInstId() == "tree2")
                setState(State::skipChildren);
        }
    };

    EnumVisitor visitor;
    tree1.com<SceneSpace>().enume(visitor, BoundVol<Box>( Box().fromCenter(Vec3(1), Vec3(5)) ));

    tree1.addCom(new TreeIdMap);
    tree1.setInstId("blah");
    tree2.setInstId("blah");

    TreeNode<int> cNode;
    TreeNode<int> cNode2;

    TreeClone<TreeNode<int>> clone;
    auto& cloneNode = clone.regNode(cNode);
    clone.update();
    cNode.setData(2);
    clone.unregClone(cloneNode);
    cNode.setData(4);
    cNode.addChild(cNode2);
    clone.update();

    SceneObject* sceneObj = tree1.com<TreeIdMap>().object("blah");
    auto objs = tree1.com<TreeIdMap>().objects("blah");
    for (auto& e : objs) { sceneObj = e.second; }

    {
        struct A : public SharedObj, public SmallAllocatorObject
        {
            A()                     : SharedObj(bind_fill(&A::dealloc, this)) {}
            ~A()                    {}
            void dealloc(void* p)   { operator delete(p); }
        };

        WeakPtr<A> weak;
        {
            SharedPtr<A> ptr = new A();
            weak = ptr;
        }
        assert(!weak.lock());
    }

    UniquePtr<int[]> pu1 = new int[50];
    pu1[0] = 5;
    UniquePtr<int[]> pu2 = move(pu1);
    SharedPtr<Object> share = new SceneObject();
    {
        SharedPtr<int> pu3 = SharedPtr<int>(pu2.get(),finalize<int[]>());
        SharedPtr<const Object> share2 = const_pointer_cast<const Object>(share);
    }
    pu2.release();

    vector<UniqueLock<Mutex>> locks;
    locks.resize(10);
    for (auto& e : locks) { e = UniqueLock<Mutex>(*new Mutex(), lock::Op::defer); }
    lock::lock(locks[0], locks[1]);
    lock::lock(range(locks.begin()+2, locks.end()));

    SharedMutex mutex;
    SharedMutex::Scoped rlock(mutex);
    {
        TransferLock<decltype(rlock), SharedMutex::SharedScoped> wlock(rlock);
    }

    void* blah = SmallMemPool::inst().pool().alloc(10000);
    debug::print(sout() << SmallMemPool::inst().pool().printStats());
    SmallMemPool::inst().pool().free(blah);
    SmallMemPool::inst().pool().validate();

    debug::print(sout() << "Vec1:   "   << v1 << endl
                        << "Vec2:   "   << v2 << endl
                        << "Vec3:   "   << v3 << endl
                        << "Vec6:   "   << v6 << endl
                        << "Real:   "   << f << endl
                        << "Quat0:  "   << q0 << endl
                        << "Quat3:  "   << q3 << endl
                        << "Quat4:  "   << q4 << endl
                        << "Tm:     "   << tm << endl
                        << "Line:   "   << line << endl
                        << "Ray:    "   << ray << endl
                        << "Plane:  "   << plane << endl
                        << "Tri:    "   << tri << endl
                        << "Sphere: "   << sphere << endl
                        << "Box:    "   << box << endl
                        << "OBox:   "   << obox << endl
                        << "Cone:   "   << cone << endl
                        << "Cyl:    "   << cyl << endl
                        << "Cap:    "   << cap << endl
                        << "Persp:  "   << persp << endl
                        << "Ortho:  "   << ortho << endl
                        );

    /*
    int count = 10000;
    int dummy = 0;
    typedef bloom_filter::Key<int> Key;
    vector<Key> keys(count, Key(count, 0.01));

    BloomFilter<Key> bloom(count, 0.01);
    for (auto i : range(0, count, 4)) { keys[i].hash(i); bloom.insert(keys[i]); }
    unordered_set<Key> set;
    for (auto i : range(0, count, 4)) { set.insert(keys[i]); }

    int64 timeBegin, timeEnd, timeFreq;
    QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&timeFreq));

    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&timeBegin));
    for (int i = 0; i < 10000000; ++i)
    {
        if (bloom.contains(keys[i%count])) ++dummy;
    }
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&timeEnd));
    debug::print(sout() << "Time 0: " << Real(timeEnd-timeBegin)/Real(timeFreq) << endl);

    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&timeBegin));
    for (int i = 0; i < 10000000; ++i)
    {
        if (set.count(keys[i%count])) ++dummy;
    }
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&timeEnd));
    debug::print(sout() << "Time 1: " << dummy << " " << Real(timeEnd-timeBegin)/Real(timeFreq) << endl);
    */
}

}
