#ifndef SOLVE_HPP
#define SOLVE_HPP

#include <seq2map/common.hpp>

namespace seq2map
{
    template<typename T> class Vectorisable
    {
    public:
        typedef std::vector<T> Vec;
        virtual Vec ToVector() const = 0;
        virtual bool FromVector(const Vec& v) = 0;
        virtual size_t GetDimension() const = 0;
    };

    typedef Vectorisable<float>  VectorisableF;
    typedef Vectorisable<double> VectorisableD;

    /**
     * Non-linear Least Squares Problem
     */
    class LeastSquaresProblem
    {
    public:
        LeastSquaresProblem(size_t m, size_t n, const Indices& vars = Indices(), double dx = 1e-3, size_t diffThreads = 0)
        : m_conds(m), m_vars(n), m_diffStep(dx),
          m_diffThreads(diffThreads > 0 ? diffThreads : GetHardwareConcurrency())
          { if (vars.empty() || !SetActiveVars(vars)) m_varIdx = makeIndices(0,n-1); }
        virtual VectorisableD::Vec Initialise() = 0;
        virtual VectorisableD::Vec Evaluate(const VectorisableD::Vec& x) const = 0;
        virtual VectorisableD::Vec Evaluate(const VectorisableD& vec) { return Evaluate(vec.ToVector()); }
        virtual cv::Mat ComputeJacobian(const VectorisableD::Vec& x, VectorisableD::Vec& y) const;
        virtual bool SetSolution(const VectorisableD::Vec& x) = 0;
        bool SetActiveVars(const Indices& varIdx);
        cv::Mat ApplyUpdate(const VectorisableD::Vec& x, const VectorisableD::Vec& delta);

    protected:
        size_t  m_conds;
        size_t  m_vars;
        Indices m_varIdx; // list of active variables
        double  m_diffStep;
        size_t  m_diffThreads;
        cv::Mat m_jacobianPattern;

    private:
        struct JacobianSlice
        {
            size_t var;
            VectorisableD::Vec x;
            VectorisableD::Vec y;
            cv::Mat col;
            cv::Mat mask;
        };

        typedef std::vector<JacobianSlice> JacobianSlices;

        static size_t GetHardwareConcurrency();
        static void DiffThread(const LeastSquaresProblem* lsq, JacobianSlices& slices);
    };

    /**
     * Levenberg-Marquardt algorithm
     */
    class LevenbergMarquardtAlgorithm
    {
    public:
        LevenbergMarquardtAlgorithm(double eta = 10.0f, double lambda = 0.0f, bool verbose = false, 
            const cv::TermCriteria& term = cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 100, 1e-6))
            : m_eta(eta), m_lambda(lambda), m_verbose(verbose), m_term(term) {}
        inline void SetVervbose(bool verbose) { m_verbose = verbose; }
        bool Solve(LeastSquaresProblem& problem, const VectorisableD::Vec& x0);
    protected:
        double m_eta;
        double m_lambda;
        bool   m_verbose;
        cv::TermCriteria m_term;
    };
}
#endif // SOLVE_HPP