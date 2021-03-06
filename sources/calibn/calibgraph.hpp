#ifndef CALIBGRAPH_HPP
#define CALIBGRAPH_HPP
#include <seq2map/common.hpp>
#include <seq2map/geometry.hpp>

using namespace seq2map;

class CalibGraph : public Persistent<Path>
{
public:
    friend class CalibGraphBuilder;
    friend class CalibGraphBundler;

    /* ctor */ CalibGraph() : m_cams(0), m_views(0) {}
	bool Create(size_t cams, size_t views, size_t refCamIdx = 0);
	bool SetReporting(const Path& docPath, const String& gpPath);
	bool Calibrate(bool pairwiseOptim);
	bool Optimise(size_t iter, double eps, size_t threads);
    bool WriteParams(const Path& calPath) const;
    bool WriteMFile(const Path& mfilePath) const;
    void Summary() const;
    virtual bool Store(const Path& path) const { Path p = path; return Store(p); }
    virtual bool Store(Path& path) const;
    virtual bool Restore(const Path& path);

private:
    class Vertex : public Indexed
    {
    public:
        Vertex(size_t idx) : Indexed(idx), initialised(false), connected(false) {}
        bool initialised;
        bool connected;
    };

    class CameraVertex : public Vertex
    {
    public:
        typedef boost::shared_ptr<CameraVertex> Ptr;
        typedef std::vector<Ptr> Ptrs;

        CameraVertex(size_t idx) : Vertex(idx)  {}
        BouguetModel       intrinsics;
        EuclideanTransform extrinsics;
        cv::Size           imageSize;
    };

    class ViewVertex : public Vertex
    {
    public:
        typedef boost::shared_ptr<ViewVertex> Ptr;
        typedef std::vector<Ptr> Ptrs;

        ViewVertex(size_t idx) : Vertex(idx) {}

        EuclideanTransform pose;
        Points3F           objectPoints;
    };

    class Observation
    {
    public:
        Observation() : rpe(-1), m_active(false) {}
        Path               source;
        Points2F           imagePoints;
        EuclideanTransform pose;
        double             rpe;

        inline bool IsActive() const { return m_active; }
        inline void SetActive(bool active) { m_active = active && !imagePoints.empty(); }

    protected:
        bool m_active;
    };

	class Report
	{
	public:
		/* ctor */ Report() {}
		/* dtor */ virtual ~Report() { Clear(); }
		bool Create(const Path& to);
		void Clear();
		void SetGnuplotPath(const Path& gnuplotPath) const;
		inline bool IsOkay() const { return m_stream.is_open(); }

	private:
		void WriteHeader();
		void WriteParams();
        void WriteOptPlot();
        void WriteRpePlots();
        void WriteFooter();

		std::ofstream m_stream;
		/*
		Path				m_inputPath;
		Path				m_reportPath;
		String		    	m_outImgPath;
		String			    m_outPlotPath;
		int					m_fontFace;
		double				m_fontScale;
		cv::Scalar			m_frontColour;
		cv::Scalar			m_backColour;
		cv::Scalar			m_colour1;
		cv::Scalar			m_colour2;
		int					m_markerSize;
		*/

		static const String s_outDocFileName;
		static const String	s_outImgDirName;
		static const String	s_outPlotDirName;
	};

    inline Observation& GetObservation(size_t cam, size_t view)             { return m_observations[cam + view * m_camvtx.size()]; }
    inline const Observation& GetObservation(size_t cam, size_t view) const { return m_observations[cam + view * m_camvtx.size()]; }
    cv::Mat GetVisibilityMatrix() const;

    typedef std::vector<Observation> Observations;

	size_t m_cams;
	size_t m_views;
    size_t m_refCamIdx;
    CameraVertex::Ptrs m_camvtx;
    ViewVertex::Ptrs   m_viewvtx;
    Observations       m_observations;
	Report             m_report;
};

#endif // CALIBGRAPH_HPP
