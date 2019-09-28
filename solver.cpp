#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <queue>
#include <chrono>
#include <iomanip>
#include <limits>
#include <random>
#include <stack>
#include <set>
#include <cstdlib>
#include <numeric>

namespace apto
{
    using PotentialType = std::uint8_t;
    using TotalPotentialType = std::uint32_t;
    using DistanceType = std::uint16_t;
    using NodeId = std::int16_t;
    using JewelId = std::int16_t;
    using SccId = std::int16_t;
    using MoveId = std::int16_t;
    using CoordsValueType = std::int16_t;

    constexpr NodeId invalidNodeId = -1;
    constexpr JewelId invalidJewelId = -1;
    constexpr SccId invalidSccId = -1;
    constexpr MoveId invalidMoveId = -1;

    constexpr int abs(int v)
    {
        return v < 0 ? -v : v;
    }

    constexpr int sign(int v)
    {
        return (v > 0) - (v < 0);
    }

    struct Coords2
    {
        CoordsValueType x, y;

        Coords2() :
            x(0),
            y(0)
        {
        }

        Coords2(int x, int y) :
            x(x),
            y(y)
        {
        }

        Coords2& operator+=(const Coords2& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }

        Coords2& operator-=(const Coords2& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }

        Coords2& operator*=(int n)
        {
            x *= n;
            y *= n;
            return *this;
        }

        friend Coords2 operator+(const Coords2& lhs, const Coords2& rhs)
        {
            return Coords2(lhs.x + rhs.x, lhs.y + rhs.y);
        }

        friend Coords2 operator-(const Coords2 & lhs, const Coords2 & rhs)
        {
            return Coords2(lhs.x - rhs.x, lhs.y - rhs.y);
        }

        friend Coords2 operator*(const Coords2 & lhs, int n)
        {
            return Coords2(lhs.x * n, lhs.y * n);
        }

        friend Coords2 operator-(const Coords2 & lhs)
        {
            return Coords2(-lhs.x, -lhs.y);
        }

        friend bool operator==(const Coords2 & lhs, const Coords2 & rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y;
        }

        friend bool operator!=(const Coords2 & lhs, const Coords2 & rhs)
        {
            return !operator==(lhs, rhs);
        }

        Coords2 sign() const
        {
            return Coords2(apto::sign(x), apto::sign(y));
        }
    };

    // stored in column-major order
    // ie. A[x][0..height-1] is contiguous
    template <typename T>
    struct Array2
    {
        Array2() :
            m_width(0),
            m_height(0),
            m_values(nullptr)
        {

        }

        Array2(int width, int height, const T& value = T{}) :
            m_width(width),
            m_height(height),
            m_values(new T[width * height])
        {
            std::fill(begin(), end(), value);
        }

        Array2(const Array2& other) :
            m_width(other.m_width),
            m_height(other.m_height),
            m_values(new T[other.m_width * other.m_height])
        {
            std::copy(std::begin(other), std::end(other), begin());
        }

        Array2(Array2&& other) noexcept :
            m_width(other.m_width),
            m_height(other.m_height),
            m_values(std::move(other.m_values))
        {
        }

        Array2& operator=(const Array2& other)
        {
            m_width = other.m_width;
            m_height = other.m_height;
            m_values.reset(new T[m_width * m_height]);
            std::copy(std::begin(other), std::end(other), begin());
            return *this;
        }

        Array2& operator=(Array2&& other)
        {
            m_width = other.m_width;
            m_height = other.m_height;
            m_values = std::move(other.m_values);
            return *this;
        }

        void fill(const T& v)
        {
            for (int i = 0; i < m_width * m_height; ++i)
            {
                m_values[i] = v;
            }
        }

        T* begin()
        {
            return m_values.get();
        }

        const T* begin() const
        {
            return m_values.get();
        }

        T* end()
        {
            return m_values.get() + m_width * m_height;
        }

        const T* end() const
        {
            return m_values.get() + m_width * m_height;
        }

        T* operator[](int x)
        {
            return begin() + x * m_height;
        }

        const T* operator[](int x) const
        {
            return begin() + x * m_height;
        }

        T& operator[](const Coords2& c)
        {
            return (*this)[c.x][c.y];
        }

        const T& operator[](const Coords2& c) const
        {
            return (*this)[c.x][c.y];
        }

        int width() const
        {
            return m_width;
        }

        int height() const
        {
            return m_height;
        }

    private:
        int m_width;
        int m_height;
        std::unique_ptr<T[]> m_values;
    };

    template <typename T, int N>
    struct SmallVector
    {
        static_assert(N > 0 && N < 256, "");

    private:
        using StorageType = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

    public:

        using value_type = T;
        using size_type = int;
        using difference_type = int;
        using reference = T &;
        using const_reference = const T &;
        using pointer = T *;
        using const_pointer = const T*;
        using iterator = T *;
        using const_iterator = const T*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        SmallVector() :
            m_size(0)
        {
        }

        SmallVector(int s) :
            m_size(s)
        {
            for (int i = 0; i < s; ++i)
            {
                new (&(m_data[i])) T{};
            }
        }

        SmallVector(int s, const T& v) :
            m_size(s)
        {
            for (int i = 0; i < s; ++i)
            {
                new (&(m_data[i])) T(v);
            }
        }

        SmallVector(const SmallVector& other) :
            m_size(other.m_size)
        {
            for (int i = 0; i < m_size; ++i)
            {
                new (&(m_data[i])) T(other[i]);
            }
        }

        SmallVector(SmallVector&& other) :
            m_size(other.m_size)
        {
            for (int i = 0; i < m_size; ++i)
            {
                new (&(m_data[i])) T(std::move(other[i]));
            }
        }

        ~SmallVector()
        {
            clear();
        }

        SmallVector& operator=(const SmallVector& other)
        {
            clear();

            m_size = other.m_size;
            for (int i = 0; i < m_size; ++i)
            {
                new (&(m_data[i])) T(other[i]);
            }
        }

        SmallVector& operator=(SmallVector&& other)
        {
            clear();

            m_size = other.m_size;
            for (int i = 0; i < m_size; ++i)
            {
                new (&(m_data[i])) T(std::move(other[i]));
            }
        }

        pointer data()
        {
            return reinterpret_cast<T*>(&(m_data[0]));
        }

        const_pointer data() const
        {
            return reinterpret_cast<const T*>(&(m_data[0]));
        }

        reference operator[](int i)
        {
            return (data()[i]);
        }

        const_reference operator[](int i) const
        {
            return (data()[i]);
        }

        reference front()
        {
            return (*this)[0];
        }

        const_reference front() const
        {
            return (*this)[0];
        }

        reference back()
        {
            return (*this)[m_size - 1];
        }

        const_reference back() const
        {
            return (*this)[m_size - 1];
        }

        iterator begin()
        {
            return data();
        }

        iterator end()
        {
            return data() + m_size;
        }

        const_iterator begin() const
        {
            return data();
        }

        const_iterator end() const
        {
            return data() + m_size;
        }

        const_iterator cbegin() const
        {
            return data();
        }

        const_iterator cend() const
        {
            return data() + m_size;
        }

        reverse_iterator rbegin()
        {
            return reverse_iterator(end());
        }

        reverse_iterator rend()
        {
            return reverse_iterator(begin());
        }

        const_reverse_iterator rbegin() const
        {
            return const_reverse_iterator(end());
        }

        const_reverse_iterator rend() const
        {
            return const_reverse_iterator(begin());
        }

        const_reverse_iterator crbegin() const
        {
            return const_reverse_iterator(cend());
        }

        const_reverse_iterator crend() const
        {
            return const_reverse_iterator(cbegin());
        }

        bool empty() const
        {
            return m_size == 0;
        }

        size_type size() const
        {
            return m_size;
        }

        size_type capacity() const
        {
            return N;
        }

        void clear()
        {
            for (int i = 0; i < m_size; ++i)
            {
                (*this)[i].~T();
            }
            m_size = 0;
        }

        template<typename... ArgsTs>
        reference emplace_back(ArgsTs&& ... args)
        {
            new (&(m_data[m_size])) T(std::forward<ArgsTs>(args)...);
            ++m_size;
            return back();
        }

        reference push_back(const T& value)
        {
            new (&(m_data[m_size])) T(value);
            ++m_size;
            return back();
        }

        reference push_back(T&& value)
        {
            new (&(m_data[m_size])) T(std::move(value));
            ++m_size;
            return back();
        }

        void pop_back()
        {
            --m_size;
            (*this)[m_size].~T();
        }

    private:
        size_type m_size;
        StorageType m_data[N];
    };

    template <typename T, typename Func>
    void forEach(Array2<T> & a, Func func)
    {
        const int width = a.width();
        const int height = a.height();
        for (int x = 0; x < width; ++x)
        {
            for (int y = 0; y < height; ++y)
            {
                func(a[x][y], x, y);
            }
        }
    }

    template <typename T, typename Func>
    void forEach(const Array2<T> & a, Func func)
    {
        const int width = a.width();
        const int height = a.height();
        for (int x = 0; x < width; ++x)
        {
            for (int y = 0; y < height; ++y)
            {
                func(a[x][y], x, y);
            }
        }
    }

    enum struct CellType : std::uint8_t
    {
        Wall,
        Jewel,
        Mine,
        Hole,
        Vehicle,
        Blank,
        Invalid
    };

    struct CellTypeHelper
    {
        static char toChar(CellType cell)
        {
            switch (cell)
            {
            case CellType::Wall:
                return '#';
            case CellType::Jewel:
                return '+';
            case CellType::Mine:
                return '*';
            case CellType::Hole:
                return 'O';
            case CellType::Vehicle:
                return '.';
            case CellType::Blank:
                return ' ';
            }
            return '\0';
        }

        static CellType fromChar(char c)
        {
            switch (c)
            {
            case '#':
                return CellType::Wall;
            case '+':
                return CellType::Jewel;
            case '*':
                return CellType::Mine;
            case 'O':
                return CellType::Hole;
            case '.':
                return CellType::Vehicle;
            case ' ':
                return CellType::Blank;
            }

            return CellType::Invalid;
        }
    };

    enum struct Direction : std::int8_t
    {
        North = 0,
        NorthEast,
        East,
        SouthEast,
        South,
        SouthWest,
        West,
        NorthWest,
        None
    };

    struct DirectionHelper
    {
        static int toId(Direction dir)
        {
            return static_cast<int>(dir);
        }

        static Direction fromId(int id)
        {
            return static_cast<Direction>(id);
        }

        static Direction rotatedClockwise(Direction dir)
        {
            return fromId((toId(dir) + 1) % 8);
        }

        static Direction rotatedCounterClockwise(Direction dir)
        {
            return fromId((toId(dir) + 7) % 8);
        }

        static Direction oppositeTo(Direction dir)
        {
            return fromId((toId(dir) + 4) % 8);
        }

        static bool areOpposite(Direction d1, Direction d2)
        {
            const int diff = toId(d1) - toId(d2);
            return abs(diff) == 4;
        }

        static bool areParallel(Direction d1, Direction d2)
        {
            const int diff = toId(d1) - toId(d2);
            return diff == 0 || abs(diff) == 4;
        }

        static bool arePerpendicular(Direction d1, Direction d2)
        {
            const int diff = toId(d1) - toId(d2);
            return abs(diff) == 2 || abs(diff) == 6;
        }

        static bool isDiagonal(Direction d)
        {
            return toId(d) & 1;
        }

        static Coords2 offset(Direction dir)
        {
            static const std::array<Coords2, 9> offsets{
                Coords2(0, -1),
                Coords2(1, -1),
                Coords2(1, 0),
                Coords2(1, 1),
                Coords2(0, 1),
                Coords2(-1, 1),
                Coords2(-1, 0),
                Coords2(-1, -1),
                Coords2(0, 0)
            };

            return offsets[toId(dir)];
        }

        static const std::array<Direction, 8> & values()
        {
            static const std::array<Direction, 8> v{
                Direction::North,
                Direction::NorthEast,
                Direction::East,
                Direction::SouthEast,
                Direction::South,
                Direction::SouthWest,
                Direction::West,
                Direction::NorthWest
            };

            return v;
        }

        static std::string toString(Direction dir)
        {
            static const std::array<std::string, 9> names{
                "North",
                "NorthEast",
                "East",
                "SouthEast",
                "South",
                "SouthWest",
                "West",
                "NorthWest",
                "None"
            };

            return names[toId(dir)];
        }

        static Direction fromDisplacement(const Coords2 & start, const Coords2 & end)
        {
            // assumes the offset lies exactly in one of the directions
            constexpr Direction directions[3][3] = {
                { Direction::NorthWest, Direction::North, Direction::NorthEast },
                { Direction::West,      Direction::None , Direction::East },
                { Direction::SouthWest, Direction::South, Direction::SouthEast }
            };

            const Coords2& offset = (end - start).sign();
            // x, y are swapped because that's how arrays work (the directions one)
            return directions[offset.y + 1][offset.x + 1];
        }
    };

    struct Board
    {
        Board(int width, int height) :
            m_cells(width, height, CellType::Invalid)
        {

        }

        CellType* operator[](int i)
        {
            return m_cells[i];
        }

        const CellType* operator[](int i) const
        {
            return m_cells[i];
        }

        CellType& operator[](const Coords2& c)
        {
            return m_cells[c];
        }

        const CellType& operator[](const Coords2& c) const
        {
            return m_cells[c];
        }

        int width() const
        {
            return m_cells.width();
        }

        int height() const
        {
            return m_cells.height();
        }

        bool isValid() const
        {
            // TODO: maybe validation
            return true;
        }

        Coords2 vehicleCoords() const
        {
            Coords2 coords(-1, -1);
            forEach([&coords](const CellType & cell, int x, int y) {
                if (cell == CellType::Vehicle)
                {
                    coords = Coords2(x, y);
                }
                });

            return coords;
        }

        template <typename FuncT>
        void forEach(FuncT&& func)
        {
            apto::forEach(m_cells, std::forward<FuncT>(func));
        }

        template <typename FuncT>
        void forEach(FuncT&& func) const
        {
            apto::forEach(m_cells, std::forward<FuncT>(func));
        }

    private:
        Array2<CellType> m_cells;
    };

    struct Level
    {
        Level(Board board, int maxMoves) :
            m_board(std::move(board)),
            m_maxMoves(maxMoves)
        {

        }

        Board& board()
        {
            return m_board;
        }

        const Board& board() const
        {
            return m_board;
        }

        int maxMoves() const
        {
            return m_maxMoves;
        }

        void setMaxMoves(int m)
        {
            m_maxMoves = m;
        }

        CellType* operator[](int i)
        {
            return m_board[i];
        }

        const CellType* operator[](int i) const
        {
            return m_board[i];
        }

        CellType& operator[](const Coords2& c)
        {
            return m_board[c];
        }

        const CellType& operator[](const Coords2& c) const
        {
            return m_board[c];
        }

        int width() const
        {
            return m_board.width();
        }

        int height() const
        {
            return m_board.height();
        }

        Coords2 vehicleCoords() const
        {
            return m_board.vehicleCoords();
        }

    private:
        Board m_board;
        int m_maxMoves;
    };

    template <typename T>
    T read(std::istream & in);

    template <>
    Level read(std::istream & in)
    {
        int width, height, maxMoves;
        in >> height >> width >> maxMoves;
        Board board(width, height);

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width;)
            {
                char c = in.get();
                CellType cell = CellTypeHelper::fromChar(c);
                if (cell == CellType::Invalid)
                {
                    continue;
                }

                board[x][y] = cell;
                ++x;
            }
        }

        return Level(std::move(board), maxMoves);
    }

    void write(const Level & level, std::ostream & out)
    {
        const int width = level.width();
        const int height = level.height();

        out << height << ' ' << width << ' ' << level.maxMoves() << '\n';
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                out << CellTypeHelper::toChar(level[x][y]);
            }
            out << '\n';
        }
    }

    struct Solution
    {
        using const_iterator = typename std::vector<Direction>::const_iterator;
        using const_reverse_iterator = typename std::vector<Direction>::const_reverse_iterator;

        static Solution empty()
        {
            return Solution(true);
        }

        static Solution invalid()
        {
            return Solution(false);
        }

        const Direction& operator[](int i) const
        {
            return m_moves[i];
        }

        const_iterator begin() const
        {
            return std::begin(m_moves);
        }

        const_iterator end() const
        {
            return std::end(m_moves);
        }

        const_reverse_iterator rbegin() const
        {
            return m_moves.rbegin();
        }

        const_reverse_iterator rend() const
        {
            return m_moves.rend();
        }

        void push(Direction dir)
        {
            m_moves.emplace_back(dir);
        }

        void append(const std::vector<Direction>& path)
        {
            m_moves.insert(std::end(m_moves), std::begin(path), std::end(path));
        }

        Direction pop()
        {
            Direction dir = m_moves.back();
            m_moves.pop_back();
            return dir;
        }

        bool exists() const
        {
            return m_exists;
        }

        bool isEmpty() const
        {
            return m_moves.empty();
        }

        void clear()
        {
            m_moves.clear();
        }

        int size() const
        {
            return m_moves.size();
        }

        void setExists(bool exists)
        {
            m_exists = exists;
        }

        bool isBetterThan(const Solution& other) const
        {
            return !other.exists() || size() < other.size();
        }

        // replacement must not be longer than length
        void replace(int start, int length, const std::vector<Direction>& replacement)
        {
            const int diff = length - static_cast<int>(replacement.size());
            m_moves.erase(std::begin(m_moves) + start, std::begin(m_moves) + start + diff);
            for (int i = 0; i < replacement.size(); ++i)
            {
                m_moves[start + i] = replacement[i];
            }
        }

    private:
        bool m_exists;
        std::vector<Direction> m_moves;

        Solution(bool exists) :
            m_exists(exists),
            m_moves()
        {
        }
    };

    void write(const Solution & solution, std::ostream & out)
    {
        if (!solution.exists())
        {
            out << "BRAK";
        }
        else
        {
            for (Direction dir : solution)
            {
                if (dir != Direction::None)
                {
                    out << DirectionHelper::toId(dir);
                }
            }
        }
    }

    struct JewelState
    {
        JewelState(int numJewels) :
            m_numCollected(numJewels, 0),
            m_isCollectible(numJewels, true),
            m_numLeft(numJewels)
        {
        }

        int numCollected(int i) const
        {
            return m_numCollected[i];
        }

        bool isCollected(int i) const
        {
            return m_numCollected[i] > 0;
        }

        // returns true only of the real collected/uncollected state changed
        bool addToCollected(int i)
        {
            m_numCollected[i] += 1;
            if (m_numCollected[i] == 1)
            {
                m_isCollectible[i] = false;
                --m_numLeft;
                return true;
            }
            return false;
        }

        // returns true only of the real collected/uncollected state changed
        bool removeFromCollected(int i)
        {
            m_numCollected[i] -= 1;
            if (m_numCollected[i] == 0)
            {
                m_isCollectible[i] = true;
                ++m_numLeft;
                return true;
            }
            return false;
        }

        int numLeft() const
        {
            return m_numLeft;
        }

        void clear()
        {
            const int numJewels = m_numCollected.size();

            for (int i = 0; i < numJewels; ++i)
            {
                m_numCollected[i] = 0;
                m_isCollectible[i] = true;
            }
            m_numLeft = numJewels;
        }

        int numJewels() const
        {
            return m_numCollected.size();
        }

    private:
        // one move can only pick up one jewel so MoveId type gives us enough storage space
        std::vector<MoveId> m_numCollected;
        std::vector<std::uint8_t> m_isCollectible;
        int m_numLeft;
    };

    struct Move
    {
        Move() = default;

        Move(int id, const Coords2& start, const Coords2& end, std::vector<JewelId> jewels) :
            m_id(id),
            m_start(start),
            m_end(end),
            m_jewels(std::move(jewels))
        {
        }

        Move(const Move&) = default;
        Move(Move&&) = default;

        Move& operator=(const Move&) = default;
        Move& operator=(Move&&) = default;

        int id() const
        {
            return m_id;
        }

        const Coords2& startPos() const
        {
            return m_start;
        }

        const Coords2& endPos() const
        {
            return m_end;
        }

        const std::vector<JewelId>& jewels() const
        {
            return m_jewels;
        }

        int numUncollectedJewelsOnTheWay(const JewelState& jewelState) const
        {
            int i = 0;
            for (int jewel : m_jewels)
            {
                if (!jewelState.isCollected(jewel))
                {
                    ++i;
                }
            }
            return i;
        }

        Direction direction() const
        {
            return DirectionHelper::fromDisplacement(m_start, m_end);
        }

        void setId(int id)
        {
            m_id = id;
        }

    private:
        Coords2 m_start;
        Coords2 m_end;
        MoveId m_id;
        std::vector<JewelId> m_jewels;
    };

    struct Moves
    {
        Moves() = default;

        Moves(const Coords2& source)
        {
            std::fill(std::begin(m_ends), std::end(m_ends), Move(invalidMoveId, source, source, {}));
        }

        Move& operator[](Direction dir)
        {
            return m_ends[DirectionHelper::toId(dir)];
        }

        const Move& operator[](Direction dir) const
        {
            return m_ends[DirectionHelper::toId(dir)];
        }

    private:
        std::array<Move, 8> m_ends;
    };

    struct Bench
    {
        using time_point = typename std::chrono::high_resolution_clock::time_point;
        using duration = typename time_point::duration;

        Bench() :
            m_numNodes(0),
            m_start{},
            m_end{}
        {

        }

        void start()
        {
            m_start = std::chrono::high_resolution_clock::now();
        }

        void end()
        {
            m_end = std::chrono::high_resolution_clock::now();
        }

        void node()
        {
            ++m_numNodes;
        }

        duration elapsed() const
        {
            return m_end - m_start;
        }

        duration elapsedToNow() const
        {
            return std::chrono::high_resolution_clock::now() - m_start;
        }

        std::uint64_t nodes() const
        {
            return m_numNodes;
        }

        double nodesPerSecond() const
        {
            return m_numNodes / static_cast<double>(elapsed().count()) * 1e9;
        }

    private:
        std::uint64_t m_numNodes;
        time_point m_start;
        time_point m_end;
    };

    bool isPerfectSquare(int n)
    {
        if (n == 0 || n == 1)
        {
            return true;
        }

        for (int i = 0; i < n / 2; ++i)
        {
            int x = i * i;
            if (x == n)
            {
                return true;
            }
            else if (n < x)
            {
                return false;
            }
        }

        return false;
    }

    struct Logger
    {
        static constexpr bool enabled = false;

        template <typename ArgT, typename... ArgTs>
        void log(ArgT&& arg, ArgTs&& ... args)
        {
            if (!enabled)
            {
                return;
            }

            (*m_stream) << arg;
            log(std::forward<ArgTs>(args)...);
        }

        template <typename ArgT>
        void log(ArgT&& arg)
        {
            if (!enabled)
            {
                return;
            }

            (*m_stream) << arg;
        }

    private:
        std::ostream* m_stream = &std::cout;
    };

    Logger g_logger;

    struct Solver
    {
    private:

        struct Scc
        {
            SccId id;
            std::vector<Coords2> nodes;
            std::vector<const Move*> bridges;
            std::vector<SccId> neighbours;
            std::vector<SccId> predecessors;
            std::vector<JewelId> jewels;
        };

    public:

        using RandomNumberGeneratorType = std::mt19937_64;

        // enable/disable checks for solution validity before commiting to it
        static constexpr bool assumeCorrect = true;

        // there are two variants possible, either we can stop there later too or not
        static constexpr bool isVehicleSpotAHole = false;

        // how much the search space for opt3 moves increases with each iteration
        static constexpr float opt3WindowIncreaseFactor = 4.0f;

        static constexpr int minimalOpt3WindowSize = 16;

        static constexpr auto maxTimeForStochasticHeuristic = std::chrono::seconds{ 1 };

        static constexpr auto maxTimeForOpt3 = std::chrono::seconds{ 1 };

        static constexpr std::uint64_t rngSeed = 12345;

        // starting potential of one jewel on one edge
        static constexpr PotentialType maxPotential = std::numeric_limits<PotentialType>::max();

        // if the best move has potential below this and we made a cycle during search
        // then try going to the nearest jewel instead
        static constexpr TotalPotentialType uncertainPotentialThreshold = 256;

        // we use probabilistic skipping in backtracking, this sets the minimal depth to use this strategy
        static constexpr int minDepthToAllowSkip = 10;

        // we keep looking for solutions of length up to maxMoves * additionalMovesFactor
        // but try to shorten ones longer than maxMoves before emmiting them as solutions
        static constexpr float additionalMovesFactor = 0.3f;

        // should be more than any possible distance on a valid board
        static constexpr DistanceType infiniteDistance = std::numeric_limits<DistanceType>::max();

        // let a be the best potential of the best edge from a node
        // if potential of the next edge is < a * pruningFactor then skip this edge
        // should be kept in [0.5, 1]
        // 0.5 means no pruning because the potential propagates with 0.5 saturation
        static constexpr float pruningFactor = 0.5f;

        Solver(Level level, Bench& bench) :
            m_rng(rngSeed),
            m_level(std::move(level)),
            m_jewelState(countJewels()),
            m_bench(&bench),

            m_vehicleCoords(m_level.vehicleCoords()),
            m_jewelIdByPosition(m_level.width(), m_level.height(), invalidJewelId),

            m_movesByPosition(m_level.width(), m_level.height()),
            m_movesByEndPosition(m_level.width(), m_level.height()),
            m_allMoves{},
            m_movesCollectingJewel(numJewels()),

            m_nodeIdByPosition(m_level.width(), m_level.height(), invalidNodeId),
            m_nodePositionById{},

            m_sccs{},
            m_lastSccWithJewel{},
            m_sccIdAt(m_level.width(), m_level.height(), invalidSccId),
            m_numSccsWithJewel{}
        {
        }

        Solution solve()
        {
            if (numJewels() == 0)
            {
                m_bench->end();
                return Solution::empty();
            }

            identifyJewels();
            g_logger.log("Recognized features\n");

            generateAllMoves();
            g_logger.log("Generated moves\n");

            if (!areAllJewelsReachable())
            {
                m_bench->end();
                return Solution::invalid();
            }

            computePairwiseNodeDistances();
            g_logger.log("Characterized vertices\n");

            identifySccs();
            g_logger.log("Sccs identified\n");

            assignJewelsToSccs();
            g_logger.log("Jewels assigned to sccs\n");

            fillSccConditionalUnreachability();
            g_logger.log("Scc unreachability filled\n");

            printSccs();

            if (!mayBeSolvable())
            {
                m_bench->end();
                g_logger.log("Unsolvable\n");
                return Solution::invalid();
            }

            m_bench->start();

            // https://www.researchgate.net/publication/307583744_The_Traveling_Purchaser_Problem_and_its_Variants p. 14
            // http://www.fsa.ulaval.ca/personnel/renaudj/pdf/Recherche/tpp(purchaser)%20COR.pdf general
            Solution cahSolution = lookForBestSolutionUsingCahHeuristicForTime(maxTimeForStochasticHeuristic);
            g_logger.log("CAH: ", cahSolution.size(), '\n');
            if (g_logger.enabled) write(cahSolution, std::cout);
            g_logger.log("\n\n");
            if (cahSolution.exists() && cahSolution.size() <= m_level.maxMoves())
            {
                m_bench->end();
                return cahSolution;
            }

            initializeSkipProbability();

            initializeMovePotential();
            g_logger.log("Initialized potential\n");

            fillInitialMovePotential();
            g_logger.log("Filled initial potential\n");

            propagateMovePotential();
            g_logger.log("Potential propagated\n");

            summarizeMovePotential();
            g_logger.log("Potential summarized\n");

            // potential field guided search with backtracking
            m_numJewelsLeftWhenSolvingAt = Array2<JewelId>(m_level.width(), m_level.height(), numJewels() + 1);
            Solution solution = solveUsingSearchWithBacktracking(m_vehicleCoords, m_level.maxMoves() - 1, 0, m_level.maxMoves() * additionalMovesFactor);
            if (solution.exists() && solution.size() <= m_level.maxMoves())
            {
                m_bench->end();
                return solution;
            }

            m_bench->end();

            return Solution::invalid();
        }

    private:
        RandomNumberGeneratorType m_rng;
        Level m_level;
        JewelState m_jewelState;
        Bench* m_bench;

        Coords2 m_vehicleCoords;
        Array2<JewelId> m_jewelIdByPosition;

        Array2<Moves> m_movesByPosition;
        Array2<std::vector<const Move*>> m_movesByEndPosition;
        std::vector<const Move*> m_allMoves;
        std::vector<std::vector<const Move*>> m_movesCollectingJewel;

        Array2<NodeId> m_nodeIdByPosition;
        std::vector<Coords2> m_nodePositionById;

        Array2<DistanceType> m_distanceFromTo;

        std::vector<Scc> m_sccs;
        std::vector<SccId> m_lastSccWithJewel; // topologically
        Array2<SccId> m_sccIdAt;
        Array2<bool> m_ifSccTraversedThenSccUnreachable;
        std::vector<SccId> m_numSccsWithJewel;

        Array2<JewelId> m_numJewelsLeftWhenSolvingAt;

        std::vector<float> m_skipProbabilityAtDepth;

        // m_potential[jewelId][edgeId]
        Array2<PotentialType> m_potentialOfJewelAtEdge;

        // m_totalPotential[edgeId]
        std::vector<TotalPotentialType> m_totalPotentialAtEdge;

        // IMPORTANT:
        // there seems to be a bug somewhere and not all solutions are correct
        // thought it doesn't come up with any local test
        // FIX LATER
        bool isSolutionValid(const Solution& solution)
        {
            if (assumeCorrect)
            {
                return true;
            }

            std::vector<std::uint8_t> isJewelCollected(numJewels(), false);
            Coords2 pos = m_vehicleCoords;

            for (Direction dir : solution)
            {
                const Coords2 dpos = DirectionHelper::offset(dir);

                for (;;)
                {
                    const Coords2 nextPos = pos + dpos;

                    if (m_level[pos] == CellType::Invalid)
                    {
                        return false;
                    }

                    if (m_level[pos] == CellType::Jewel)
                    {
                        isJewelCollected[m_jewelIdByPosition[pos]] = true;
                    }

                    if (m_level[pos] == CellType::Mine)
                    {
                        return false;
                    }

                    if (m_level[nextPos] == CellType::Wall)
                    {
                        break;
                    }

                    pos = nextPos;

                    if (m_level[pos] == CellType::Hole || (m_level[pos] == CellType::Vehicle && isVehicleSpotAHole))
                    {
                        break;
                    }
                }
            }

            return std::all_of(std::begin(isJewelCollected), std::end(isJewelCollected), [](std::uint8_t v) {return v; });
        }

        template <typename FuncT>
        void forEachMoveInSolution(const Solution& solution, FuncT&& func) const
        {
            Coords2 start = m_vehicleCoords;
            forEachMoveInSolution(std::begin(solution), std::end(solution), start, std::forward<FuncT>(func));
        }

        template <typename FuncT>
        void forEachMoveInSolution(const std::vector<Direction>& solution, Coords2& start, FuncT&& func) const
        {
            forEachMoveInSolution(std::begin(solution), std::end(solution), start, std::forward<FuncT>(func));
        }

        template <typename IterT, typename FuncT>
        void forEachMoveInSolution(IterT begin, IterT end, Coords2& start, FuncT&& func) const
        {
            while(begin != end)
            {
                const auto& move = m_movesByPosition[start][*begin];

                func(move, start);

                start = move.endPos();
                ++begin;
            }
        }

        bool opt3(std::vector<NodeId>& nodesInPath, std::vector<int>& successors, const std::vector<std::uint8_t>& isAnyImportantJewelOnThisEdge, int window) const
        {
            // window parameter specifies the upper bound on how far the nodes being exchanged can be located to each other

            // tries to lower the overall path length by trying out all possible non path reversing
            // 3-opt moves http://akira.ruc.dk/~keld/research/LKH/LKH-2.0/DOC/LKH_REPORT.pdf p. 9

            // we have to preserve order of traversing single moves so there is always one possibility
            //   i    j    k
            // 0 -> 1 -> 2 -> 3
            // into
            // 0 -> 2 -> 1 -> 3
            //   i    j    k

            const int totalLength = solutionThroughNodes(nodesInPath, successors).size();
            int savedLength = 0;
            bool anyImprovement = false;
            for (int i0 = 0, i = 0; i0 + 5 < nodesInPath.size(); ++i0, i = successors[i])
            {
                if (totalLength - savedLength <= m_level.maxMoves())
                {
                    break;
                }

                if (isAnyImportantJewelOnThisEdge[i]) continue;
                const int iStart = nodesInPath[i];
                const int iEnd = nodesInPath[successors[i]];
                const int iCost = m_distanceFromTo[iStart][iEnd];

                for (int j0 = i0 + 2, j = successors[successors[i]]; j0 + 3 < nodesInPath.size() && j0 < i0 + window; ++j0, j = successors[j])
                {
                    if (isAnyImportantJewelOnThisEdge[j]) continue;
                    const int jStart = nodesInPath[j];
                    const int jEnd = nodesInPath[successors[j]];
                    const int jCost = m_distanceFromTo[jStart][jEnd];

                    bool anyChange = false;
                    for (int k0 = j0 + 2, k = successors[successors[j]]; k0 + 1 < nodesInPath.size() && k0 < j0 + window; ++k0, k = successors[k])
                    {
                        if (isAnyImportantJewelOnThisEdge[k]) continue;

                        const int kStart = nodesInPath[k];
                        const int kEnd = nodesInPath[successors[k]];
                        const int kCost = m_distanceFromTo[kStart][kEnd];

                        const int iStartNew = iStart;
                        const int iEndNew = jEnd;
                        const int jStartNew = kStart;
                        const int jEndNew = iEnd;
                        const int kStartNew = jStart;
                        const int kEndNew = kEnd;

                        const int iCostNew = m_distanceFromTo[iStartNew][iEndNew];
                        const int jCostNew = m_distanceFromTo[jStartNew][jEndNew];
                        const int kCostNew = m_distanceFromTo[kStartNew][kEndNew];
                        if (iCostNew == infiniteDistance || jCostNew == infiniteDistance || kCostNew == infiniteDistance)
                        {
                            continue;
                        }

                        const int cost = iCost + jCost + kCost;
                        const int costNew = iCostNew + jCostNew + kCostNew;
                        if (costNew < cost)
                        {
                            // exchange
                            //g_logger.log(iStart, ' ', iEnd, ' ', jStart, ' ', jEnd, ' ', kStart, ' ', kEnd, ' ', cost, ' ', iCost, ' ', jCost, ' ', kCost, '\n');
                            //g_logger.log(iStartNew, ' ', iEndNew, ' ', jStartNew, ' ', jEndNew, ' ', kStartNew, ' ', kEndNew, ' ', costNew, ' ', iCostNew, ' ', jCostNew, ' ', kCostNew, '\n');
                            //g_logger.log(successors[i], ' ', successors[j], ' ', successors[k], '\n');

                            const int sj = successors[j];
                            successors[j] = successors[k];
                            successors[k] = successors[i];
                            successors[i] = sj;

                            savedLength += cost - costNew;
                            g_logger.log("opt3 ", i, ": ", totalLength - savedLength, '\n');

                            anyImprovement = true;
                            anyChange = true;
                            break;
                        }
                    }
                    // we have to break to the outermost loop when a change is made
                    // because it breaks for some reason otherwise
                    // it's complex so invalidations happen when changing order of edges which causes problems
                    if (anyChange) break;
                }
            }

            return anyImprovement;
        }

        Solution solutionThroughNodes(const std::vector<NodeId>& nodes, const std::vector<int>& successors) const
        {
            Solution solution = Solution::empty();
            std::vector<Direction> pathBuffer;
            for (int i = 0, j = 0; j + 1 < nodes.size(); ++j)
            {
                const int startNodeId = nodes[i];
                const int endNodeId = nodes[successors[i]];
                pathBuffer.clear();
                shortestPathFromTo(m_nodePositionById[startNodeId], m_nodePositionById[endNodeId], pathBuffer);
                solution.append(pathBuffer);
                i = successors[i];
            }
            return solution;
        }

        Solution solutionThroughNodes(const std::vector<NodeId>& nodes) const
        {
            Solution solution = Solution::empty();
            std::vector<Direction> pathBuffer;
            for (int i = 0; i + 1 < nodes.size(); ++i)
            {
                const int startNodeId = nodes[i];
                const int endNodeId = nodes[i + 1];
                pathBuffer.clear();
                shortestPathFromTo(m_nodePositionById[startNodeId], m_nodePositionById[endNodeId], pathBuffer);
                solution.append(pathBuffer);
            }
            return solution;
        }

        void solutionToNodeList(const Solution& solution, std::vector<NodeId>& nodesInPath, std::vector<std::uint8_t>& isAnyImportantJewelOnThisEdge) const
        {
            Coords2 c = m_vehicleCoords;
            nodesInPath.emplace_back(m_nodeIdByPosition[c]);
            std::vector<std::uint8_t> isJewelCollected(numJewels(), false);
            forEachMoveInSolution(solution, [&](const Move & move, const Coords2 & pos) {
                bool anyNewJewels = false;
                for (const int jewelId : move.jewels())
                {
                    if (!isJewelCollected[jewelId])
                    {
                        isJewelCollected[jewelId] = true;
                        anyNewJewels = true;
                    }
                }

                const int endNodeId = m_nodeIdByPosition[move.endPos()];

                nodesInPath.emplace_back(endNodeId);
                isAnyImportantJewelOnThisEdge.emplace_back(anyNewJewels);
            });
        }

        void solutionToCoalescedNodeList(const Solution& solution, std::vector<NodeId>& nodesInPathCoalesced, std::vector<std::uint8_t>& isAnyImportantJewelOnThisEdgeCoalesced) const
        {
            std::vector<NodeId> nodesInPath;
            std::vector<std::uint8_t> isAnyImportantJewelOnThisEdge;
            solutionToNodeList(solution, nodesInPath, isAnyImportantJewelOnThisEdge);

            nodesInPathCoalesced.emplace_back(m_nodeIdByPosition[m_vehicleCoords]);
            for (int i = 0; i + 1 < isAnyImportantJewelOnThisEdge.size(); ++i)
            {
                if (isAnyImportantJewelOnThisEdge[i] || isAnyImportantJewelOnThisEdge[i + 1])
                {
                    nodesInPathCoalesced.emplace_back(nodesInPath[i + 1]);
                    isAnyImportantJewelOnThisEdgeCoalesced.emplace_back(isAnyImportantJewelOnThisEdge[i]);
                }
            }
            nodesInPathCoalesced.emplace_back(nodesInPath.back());
            isAnyImportantJewelOnThisEdgeCoalesced.emplace_back(isAnyImportantJewelOnThisEdge.back());
        }

        void opt3(Solution & solution) const
        {
            // prepares the date structure
            // does 3-opt moves until the solution is good enough or no improvement can be made
            // uses ever increasing window size of searching to converge faster to nearly
            // maximally improved solution. Gives an advantage on very big boards (>100x100) where one whole iteration takes too long.

            std::vector<NodeId> nodesInPathCoalesced;
            std::vector<std::uint8_t> isAnyImportantJewelOnThisEdgeCoalesced;
            solutionToCoalescedNodeList(solution, nodesInPathCoalesced, isAnyImportantJewelOnThisEdgeCoalesced);

            std::vector<int> successors(nodesInPathCoalesced.size());
            std::iota(std::begin(successors), std::end(successors), 1);

            int window = std::max(minimalOpt3WindowSize, static_cast<int>(std::sqrt(nodesInPathCoalesced.size())));
            int tries = 0;
            while (solution.size() > m_level.maxMoves())
            {
                if (!opt3(nodesInPathCoalesced, successors, isAnyImportantJewelOnThisEdgeCoalesced, window))
                {
                    break;
                }

                solution = solutionThroughNodes(nodesInPathCoalesced, successors);

                window = std::min(static_cast<int>(window * opt3WindowIncreaseFactor), static_cast<int>(nodesInPathCoalesced.size()));
            }
        }

        int numJewels() const
        {
            return m_jewelState.numJewels();
        }

        Solution lookForBestSolutionUsingCahHeuristicForTime(std::chrono::milliseconds time)
        {
            const int numMoves = m_allMoves.size();

            std::vector<int> penalties(numMoves, 0);
            std::vector<int> lastPenaltyIter(numMoves, -1);
            std::vector<int> numConsecutivePenalties(numMoves, 0);

            std::vector<Solution> bestSolutions;
            Solution best = Solution::invalid();
            int v = 0;
            int i = 0;

            // cah generator needs to know the previous best raw solution length
            // to know when to apply more costly heuristics
            int currentBestBeforeReduction = std::numeric_limits<int>::max();

            for (;;)
            {
                ++i;
                Solution solution = Solution::invalid();
                if (solveUsingCahHeuristic(solution, m_vehicleCoords, penalties, lastPenaltyIter, numConsecutivePenalties, currentBestBeforeReduction, i))
                {
                    ++v;
                    if (isSolutionValid(solution) && solution.isBetterThan(best))
                    {
                        forEachMoveInSolution(solution, [&](const Move & move, const Coords2 & pos) {
                            penalties[move.id()] -= 1;
                        });

                        bestSolutions.emplace_back(solution);
                        best = std::move(solution);
                        g_logger.log(i, ": ", best.size(), '\n');
                    }
                }

                if (best.exists() && best.size() <= m_level.maxMoves())
                {
                    return best;
                }

                if (m_bench->elapsedToNow() > maxTimeForStochasticHeuristic) break;
            }

            g_logger.log(v, '/', i, " valid CAH solutions\n");

            // try optimising all of them, starting from the most promising ones
            // this rarely gives an improvement but for large boards
            // is much more hopeful than later search and for
            // small ones it goes fast
            std::reverse(std::begin(bestSolutions), std::end(bestSolutions));
            for (Solution& sol : bestSolutions)
            {
                if (m_bench->elapsedToNow() > maxTimeForStochasticHeuristic + maxTimeForOpt3) break;

                opt3(sol);
                if (!isSolutionValid(sol))
                {
                    continue;
                }

                if (sol.size() <= m_level.maxMoves())
                {
                    m_bench->end();
                    return sol;
                }
                else if (sol.isBetterThan(best))
                {
                    best = std::move(sol);
                }
            }

            return best;
        }

        bool solveUsingCahHeuristic(
            Solution & solution,
            const Coords2 & start,
            std::vector<int>& penalties,
            std::vector<int>& lastPenaltyIter,
            std::vector<int>& numConsecutivePenalties,
            int& currentBestBeforeReduction,
            int iter)
        {
            // based on what is briefly described in
            // https://www.researchgate.net/publication/307583744_The_Traveling_Purchaser_Problem_and_its_Variants p. 14
            // along with checks regarding solvability due to possiblity of having
            // many strongly connected components
            // Also applies penalty to edges that led to blockages

            // to avoid multiple allocations
            std::vector<Direction> pathBuffer;

            auto addJewelsFromPath = [this, &pathBuffer](int startNodeId, int endNodeId)
            {
                Coords2 c = m_nodePositionById[startNodeId];
                pathBuffer.clear();
                shortestPathFromTo(m_nodePositionById[startNodeId], m_nodePositionById[endNodeId], pathBuffer);
                forEachMoveInSolution(pathBuffer, c, [&](const Move & move, const Coords2 & pos) {
                    for (const int jewelId : move.jewels())
                    {
                        m_jewelState.addToCollected(jewelId);
                    }
                });
            };

            auto removeJewelsFromPath = [this, &pathBuffer](int startNodeId, int endNodeId)
            {
                Coords2 c = m_nodePositionById[startNodeId];
                pathBuffer.clear();
                shortestPathFromTo(m_nodePositionById[startNodeId], m_nodePositionById[endNodeId], pathBuffer);
                forEachMoveInSolution(pathBuffer, c, [&](const Move & move, const Coords2 & pos) {
                    for (const int jewelId : move.jewels())
                    {
                        m_jewelState.removeFromCollected(jewelId);
                    }
                });
            };

            auto applyPenaltiesToPath = [&](const std::vector<NodeId> & path)
            {
                Coords2 c = start;
                for (int i = 0; i + 1 < path.size(); i += 1)
                {
                    const int startNodeId = path[i];
                    const int endNodeId = path[i + 1];
                    const DistanceType distance = m_distanceFromTo[startNodeId][endNodeId];
                    pathBuffer.clear();
                    shortestPathFromTo(m_nodePositionById[startNodeId], m_nodePositionById[endNodeId], pathBuffer);

                    forEachMoveInSolution(pathBuffer, c, [&](const Move & move, const Coords2 & pos) {
                        const int moveId = move.id();
                        auto old = penalties[moveId];
                        if (iter > lastPenaltyIter[moveId] + 1)
                        {
                            numConsecutivePenalties[moveId] = 0;
                        }

                        penalties[moveId] += numConsecutivePenalties[moveId] * 2 + 1;
                        ++numConsecutivePenalties[moveId];
                        lastPenaltyIter[moveId] = iter;
                    });
                }
            };

            const int numJewels = m_jewelState.numJewels();
            std::vector<JewelId> jewelIdsShuffled(numJewels);
            std::iota(std::begin(jewelIdsShuffled), std::end(jewelIdsShuffled), 0);
            std::shuffle(std::begin(jewelIdsShuffled), std::end(jewelIdsShuffled), m_rng);

            std::vector<NodeId> nodesInPath;
            nodesInPath.emplace_back(m_nodeIdByPosition[start]);

            std::vector<std::uint8_t> isTraversed(m_sccs.size(), false);
            std::vector<std::uint8_t> mayBeEnterable(m_sccs.size(), true);
            isTraversed[m_sccIdAt[start]] = true;

            auto insertForJewel = [&](int jewelId) -> int {
                const Move* bestMove = nullptr;
                int bestI = -1;
                int lowestDistance = std::numeric_limits<int>::max();
                int additionalDistance = infiniteDistance;

                for (auto move : m_movesCollectingJewel[jewelId])
                {
                    // if going to this scc would prevent us from accessing any jewel
                    // (because we would lose access to its only scc) then mark this scc as a no go
                    const int startSccId = m_sccIdAt[move->startPos()];
                    const int endSccId = m_sccIdAt[move->endPos()];
                    if (mayBeEnterable[startSccId] && !remainsSolvableAfterEnteringScc(isTraversed, startSccId))
                    {
                        mayBeEnterable[startSccId] = false;
                        continue;
                    }
                    if (mayBeEnterable[endSccId] && !remainsSolvableAfterEnteringScc(isTraversed, endSccId))
                    {
                        mayBeEnterable[endSccId] = false;
                        continue;
                    }

                    const int numJewelsOnTheWay = static_cast<int>(move->jewels().size());
                    const int moveValue = numJewelsOnTheWay - penalties[move->id()];

                    const int thisMoveStartId = m_nodeIdByPosition[move->startPos()];
                    const int thisMoveEndId = m_nodeIdByPosition[move->endPos()];

                    // we skip one edge each iteration because we have to go through it and collect the jewels
                    for (int i = 0; i + 1 < nodesInPath.size(); i += 2)
                    {
                        const int startNodeId = nodesInPath[i];
                        const int endNodeId = nodesInPath[i + 1];
                        const DistanceType d0 = m_distanceFromTo[startNodeId][thisMoveStartId];
                        const DistanceType d1 = m_distanceFromTo[thisMoveEndId][endNodeId];
                        const DistanceType dOld = m_distanceFromTo[startNodeId][endNodeId];
                        if (d0 == infiniteDistance || d1 == infiniteDistance)
                        {
                            continue;
                        }
                        const int distance = d0 + d1 - dOld - moveValue;
                        if (distance < lowestDistance)
                        {
                            bestI = i;
                            lowestDistance = distance;
                            bestMove = move;
                            additionalDistance = d0 + d1 - dOld;
                        }
                    }

                    const int dn = m_distanceFromTo[nodesInPath.back()][thisMoveStartId];
                    const int distance = dn - moveValue;
                    if (dn != infiniteDistance && distance < lowestDistance)
                    {
                        bestI = nodesInPath.size() - 1;
                        lowestDistance = distance;
                        bestMove = move;
                        additionalDistance = dn;
                    }
                }

                if (bestI < 0)
                {
                    applyPenaltiesToPath(nodesInPath);
                    m_jewelState.clear();
                    return infiniteDistance;
                }

                for (const int jewelId : bestMove->jewels())
                {
                    m_jewelState.addToCollected(jewelId);
                }

                const int bestMoveStartId = m_nodeIdByPosition[bestMove->startPos()];
                const int bestMoveEndId = m_nodeIdByPosition[bestMove->endPos()];

                isTraversed[m_sccIdAt[bestMove->startPos()]] = true;
                isTraversed[m_sccIdAt[bestMove->endPos()]] = true;

                {
                    const int startNodeId = nodesInPath[bestI];
                    addJewelsFromPath(startNodeId, bestMoveStartId);

                    if (bestI != nodesInPath.size() - 1)
                    {
                        const int endNodeId = nodesInPath[bestI + 1];
                        addJewelsFromPath(bestMoveEndId, endNodeId);
                        removeJewelsFromPath(startNodeId, endNodeId);
                    }
                }

                int e[] = { bestMoveStartId, bestMoveEndId };
                nodesInPath.insert(std::begin(nodesInPath) + bestI + 1, std::begin(e), std::end(e));

                return additionalDistance + 1;
            };

            while (m_jewelState.numLeft() > 0)
            {
                for (int a = 0; a < numJewels; ++a)
                {
                    const int jewelId = jewelIdsShuffled[a];
                    if (m_jewelState.isCollected(jewelId))
                    {
                        continue;
                    }

                    if (insertForJewel(jewelId) == infiniteDistance)
                    {
                        return false;
                    }
                }
            }

            auto tryExchange = [&](int i) -> bool {
                const int startNodeId = nodesInPath[i];
                const int leftMiddleNodeId = nodesInPath[i + 1];
                const int rightMiddleNodeId = nodesInPath[i + 2];
                int endNodeId = invalidNodeId;
                int distanceSaved = 0;
                {
                    removeJewelsFromPath(startNodeId, leftMiddleNodeId);
                    removeJewelsFromPath(leftMiddleNodeId, rightMiddleNodeId);

                    distanceSaved = m_distanceFromTo[startNodeId][leftMiddleNodeId];

                    if (i + 3 < nodesInPath.size())
                    {
                        endNodeId = nodesInPath[i + 3];
                        removeJewelsFromPath(rightMiddleNodeId, endNodeId);
                        addJewelsFromPath(startNodeId, endNodeId);

                        distanceSaved +=
                            m_distanceFromTo[rightMiddleNodeId][endNodeId]
                            - m_distanceFromTo[startNodeId][endNodeId];
                    }
                }

                nodesInPath.erase(std::begin(nodesInPath) + i + 1, std::begin(nodesInPath) + i + 3);

                int addedDistance = 0;

                while (m_jewelState.numLeft() > 0)
                {
                    for (int jewelId = 0; jewelId < numJewels; ++jewelId)
                    {
                        if (m_jewelState.isCollected(jewelId))
                        {
                            continue;
                        }

                        addedDistance += insertForJewel(jewelId);
                    }
                }

                return addedDistance < distanceSaved;
            };

            auto exchange = [&]() -> bool {
                bool anyImprovement = false;

                // we skip one edge each iteration because we have to go through it and collect the jewels
                for (int i = 0; i + 1 < nodesInPath.size(); i += 2)
                {
                    JewelState oldState = m_jewelState;
                    auto nodesInPathCpy = nodesInPath;
                    if (tryExchange(i))
                    {
                        anyImprovement = true;
                    }
                    else
                    {
                        m_jewelState = std::move(oldState);
                        nodesInPath = std::move(nodesInPathCpy);
                    }
                }

                return anyImprovement;
            };

            solution = solutionThroughNodes(nodesInPath);

            // shorten all possible subpaths. Here we do it only to get the length of the shortened solution
            {
                JewelState oldState = m_jewelState;
                while (tryRemoveAnyRunFromSolution(solution))
                {
                }
                m_jewelState = std::move(oldState);
            }

            if (solution.size() < currentBestBeforeReduction)
            {
                currentBestBeforeReduction = solution.size();

                // exchange markets until not improvement can be made
                for (;;)
                {
                    if (solution.size() <= m_level.maxMoves())
                    {
                        break;
                    }

                    if (!exchange())
                    {
                        break;
                    }
                }
            }

            // again do the same as before exchange
            solution = solutionThroughNodes(nodesInPath);
            // shorten all possible subpaths
            while (tryRemoveAnyRunFromSolution(solution))
            {
            }

            m_jewelState.clear();

            solution.setExists(true);

            return true;
        }

        void initializeSkipProbability()
        {
            const int maxMoves = m_level.maxMoves();
            m_skipProbabilityAtDepth.resize(m_level.maxMoves(), 0.0f);

            for (int i = 0; i < maxMoves; ++i)
            {
                int skip = maxMoves / std::sqrt(maxMoves - i) + 1;
                skip *= skip;
                if (i <= minDepthToAllowSkip || isPerfectSquare(i))
                {
                    m_skipProbabilityAtDepth[i] = 0.0f;
                }
                else
                {
                    m_skipProbabilityAtDepth[i] = 1.0f - 1.0f / static_cast<float>(skip);
                }
            }
        }

        void updateJewelStateOnSolutionPartReplaced(const Solution & oldSolution, int start, int length, const std::vector<Direction> & directions)
        {
            // reevaluates jewels collected on the path

            const auto starts = coordsAlongSolution(oldSolution);

            Coords2 coords = starts[start];
            for (int i = start; i < start + length; ++i)
            {
                const Moves& moves = m_movesByPosition[coords];
                const Move& move = moves[oldSolution[i]];
                for (int jewelId : move.jewels())
                {
                    m_jewelState.removeFromCollected(jewelId);
                }

                coords = move.endPos();
            }

            coords = starts[start];
            for (int i = 0; i < directions.size(); ++i)
            {
                const Moves& moves = m_movesByPosition[coords];
                const Move& move = moves[directions[i]];
                for (int jewelId : move.jewels())
                {
                    m_jewelState.addToCollected(jewelId);
                }

                coords = move.endPos();
            }
        }

        std::vector<Coords2> coordsAlongSolution(const Solution & solution) const
        {
            // [i] contains the position of the vehicle before move solution[i]

            std::vector<Coords2> r{};
            r.reserve(solution.size() + 1);

            Coords2 coords = m_vehicleCoords;
            r.emplace_back(coords);
            for (int i = 0; i < solution.size(); ++i)
            {
                const Moves& moves = m_movesByPosition[coords];
                const Move& move = moves[solution[i]];
                coords = move.endPos();
                r.emplace_back(coords);
            }

            return r;
        }

        bool tryRemoveAnyRunFromSolution(Solution & solution)
        {
            // tries to remove the run that would shorten the path the most
            // returns true if anything removed

            auto run = findMostImprovableRedundantEdgeRun(solution);

            if (run.first != -1)
            {
                const int start = run.first;
                const int length = run.second;
                auto sol = tryShortenRun(solution, start, length);
                updateJewelStateOnSolutionPartReplaced(solution, start, length, sol);
                solution.replace(start, length, sol);
                return true;
            }

            return false;
        }


        std::vector<Direction> tryShortenRun(const Solution & solution, int start, int length) const
        {
            // looks for a shorter path to replace solution[start..start+length]

            if (length <= 1) return {};

            Coords2 coords = m_vehicleCoords;
            for (int i = 0; i < start; ++i)
            {
                const Moves& moves = m_movesByPosition[coords];
                const Move& move = moves[solution[i]];
                coords = move.endPos();
            }

            Coords2 endCoords = coords;
            for (int i = start; i < start + length; ++i)
            {
                const Moves& moves = m_movesByPosition[endCoords];
                const Move& move = moves[solution[i]];
                endCoords = move.endPos();
            }

            // coords now at where we should start
            // endCoords where we should end
            std::vector<Direction> path{};
            if (pathFromToWithLength(coords, endCoords, length - 1, path))
            {
                return path;
            }

            return {};
        }

        bool shortestPathFromTo(const Coords2 & fromCoords, const Coords2 & toCoords, std::vector<Direction> & path) const
        {
            const int from = m_nodeIdByPosition[fromCoords];
            int to = m_nodeIdByPosition[toCoords];
            return pathFromToWithLength(fromCoords, toCoords, m_distanceFromTo[from][to], path);
        }

        bool pathFromToWithLength(const Coords2 & fromCoords, const Coords2 & toCoords, int length, std::vector<Direction> & path) const
        {
            // returns false if no such path exists

            int from = m_nodeIdByPosition[fromCoords];
            const int to = m_nodeIdByPosition[toCoords];
            if (m_distanceFromTo[from][to] > length)
            {
                return false;
            }

            while (from != to)
            {
                if (length-- <= 0)
                {
                    return false;
                }

                const int distance = m_distanceFromTo[from][to];

                const auto& moves = m_movesByPosition[m_nodePositionById[from]];
                int newFrom = from;
                for (Direction dir : DirectionHelper::values())
                {
                    const auto& move = moves[dir];

                    if (move.id() < 0)
                    {
                        continue;
                    }

                    const int newFromCandidate = m_nodeIdByPosition[move.endPos()];
                    if (m_distanceFromTo[newFromCandidate][to] < distance)
                    {
                        path.emplace_back(dir);
                        newFrom = newFromCandidate;
                        break;
                    }
                }

                if (from == newFrom)
                {
                    return false;
                }
                else
                {
                    from = newFrom;
                }
            }

            return true;
        }

        // the jewelState must not be cleared yet
        std::pair<int, int> findMostImprovableRedundantEdgeRun(const Solution & solution) const
        {
            struct Improvement
            {
                int start;
                int length;
                int possibleImprovement;
            };

            Improvement bestImprovement{ -1, -1, 0 };
            const auto starts = coordsAlongSolution(solution);
            const int solLength = solution.size();
            std::vector<MoveId> numOmitted(m_jewelState.numJewels(), 0);
            int begin = 0;
            int end = 0;
            while (begin != solLength && end != solLength)
            {
                while (end != solLength)
                {
                    const Coords2& coords = starts[end];
                    const Moves& moves = m_movesByPosition[coords];
                    const Move& move = moves[solution[end]];
                    bool isRedundant = true;
                    for (int jewelId : move.jewels())
                    {
                        if (m_jewelState.numCollected(jewelId) - numOmitted[jewelId] < 2)
                        {
                            isRedundant = false;
                        }

                        ++numOmitted[jewelId];
                    }

                    if (!isRedundant)
                    {
                        // backtrack incrementing ommitted count
                        for (int jewelId : move.jewels())
                        {
                            --numOmitted[jewelId];
                        }

                        break;
                    }

                    ++end;
                }

                if (begin != end)
                {
                    const int start = begin;
                    const int length = end - begin;
                    if (length > bestImprovement.possibleImprovement)
                    {
                        const Coords2 fromCoords = starts[start];
                        const Coords2 toCoords = starts[start + length];
                        const int from = m_nodeIdByPosition[fromCoords];
                        const int to = m_nodeIdByPosition[toCoords];
                        const DistanceType newLength = m_distanceFromTo[from][to];
                        const int impr = length - newLength;
                        if (impr > bestImprovement.possibleImprovement)
                        {
                            bestImprovement.start = start;
                            bestImprovement.length = length;
                            bestImprovement.possibleImprovement = impr;
                        }
                    }
                }

                const Coords2& coords = starts[begin];
                const Moves& moves = m_movesByPosition[coords];
                const Move& move = moves[solution[begin]];
                for (int jewelId : move.jewels())
                {
                    --numOmitted[jewelId];
                }

                ++begin;
            }

            return { bestImprovement.start, bestImprovement.length };
        }

        SmallVector<const Move*, 8> orderMoves(const Moves & moves) const
        {
            SmallVector<const Move*, 8> dirs;

            for (Direction dir : DirectionHelper::values())
            {
                const Move& move = moves[dir];

                if (move.id() < 0)
                {
                    continue;
                }

                const Coords2& start = move.startPos();
                const Coords2& end = move.endPos();
                const int startSccId = m_sccIdAt[start];
                const int endSccId = m_sccIdAt[end];
                if (startSccId != endSccId && !canMoveToScc(endSccId))
                {
                    continue;
                }

                dirs.emplace_back(&moves[dir]);
            }

            std::sort(std::begin(dirs), std::end(dirs), [this, &moves](const Move * lhs, const Move * rhs) {
                return m_totalPotentialAtEdge[lhs->id()] > m_totalPotentialAtEdge[rhs->id()];
                });

            return dirs;
        }

        bool canMoveToScc(int id) const
        {
            // there are possibilties of false positives
            // but they should be rare and only impact performance
            // ability to find a solution is unharmed

            if (id == invalidSccId)
            {
                return false;
            }

            const int numJewels = m_jewelState.numJewels();
            for (int jewelId = 0; jewelId < numJewels; ++jewelId)
            {
                if (id > m_lastSccWithJewel[jewelId])
                {
                    // we must already have this jewel, because we can't pick it later
                    if (!m_jewelState.isCollected(jewelId))
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        const Move* findNearestMoveWithUncollectedJewel(const Coords2 & start)
        {
            const int numJewels = m_jewelState.numJewels();

            const int startNodeId = m_nodeIdByPosition[start];

            const Move* bestMove = nullptr;
            int bestMoveDistance = std::numeric_limits<int>::max();
            for (int jewelId = 0; jewelId < numJewels; ++jewelId)
            {
                if (m_jewelState.isCollected(jewelId))
                {
                    continue;
                }

                for (const Move* move : m_movesCollectingJewel[jewelId])
                {
                    const int startSccId = m_sccIdAt[move->startPos()];
                    const int endSccId = m_sccIdAt[move->endPos()];
                    if (startSccId != endSccId && !canMoveToScc(endSccId))
                    {
                        continue;
                    }

                    const int moveStartNodeId = m_nodeIdByPosition[move->startPos()];
                    const DistanceType distance = m_distanceFromTo[startNodeId][moveStartNodeId];
                    if (distance < bestMoveDistance)
                    {
                        bestMoveDistance = distance;
                        bestMove = move;
                    }
                }
            }

            return bestMove;
        }

        Solution solveUsingSearchWithBacktracking(const Coords2& coords, int movesLeft, int depth, int additionalMoves)
        {
            int minDepth = 0;
            Solution solution = Solution::empty();
            if (solveUsingSearchWithBacktracking(solution, coords, movesLeft, depth, additionalMoves, minDepth))
            {
                return solution;
            }
            return Solution::invalid();
        }

        bool solveUsingSearchWithBacktracking(Solution & solution, const Coords2 & coords, int movesLeft, int depth, int additionalMoves, int& minDepth)
        {
            m_bench->node();

            if (depth < minDepth)
            {
                minDepth = depth;
            }

            const Moves& moves = m_movesByPosition[coords];

            const auto orderedMoves = orderMoves(moves);
            if (orderedMoves.empty())
            {
                return false;
            }

            const float maxPotential = static_cast<float>(m_totalPotentialAtEdge[orderedMoves[0]->id()]);
            const float potentialThreshold = maxPotential * pruningFactor;
            for (const Move* movePtr : orderedMoves)
            {
                const Move& move = *movePtr;

                const float potential = static_cast<float>(m_totalPotentialAtEdge[move.id()]);
                if (potential < potentialThreshold)
                {
                    return false;
                }

                const Coords2& end = move.endPos();

                const int oldNumJewelsLeft = m_numJewelsLeftWhenSolvingAt[end];
                if (m_jewelState.numLeft() - move.numUncollectedJewelsOnTheWay(m_jewelState) >= oldNumJewelsLeft)
                {
                    // we have not made progress and we are in the
                    // same state as when we were here previously

                    // if potential is not very well defined here then try to move to the
                    // nearest edge that collects a new jewel

                    // TODO: fill m_numJewelsLeftWhenSolvingAt correctly
                    if (potential < uncertainPotentialThreshold)
                    {
                        const Move* bestMove = findNearestMoveWithUncollectedJewel(coords);
                        if (bestMove == nullptr)
                        {
                            return false;
                        }

                        std::vector<Direction> path;
                        // this appends twice to the same path, effectively joining them
                        if (shortestPathFromTo(coords, bestMove->startPos(), path) && shortestPathFromTo(bestMove->startPos(), bestMove->endPos(), path))
                        {
                            if (path.size() > -additionalMoves)
                            {
                                return false;
                            }

                            Coords2 newCoords = coords;
                            forEachMoveInSolution(path, newCoords, [&](const Move & move, const Coords2 & coords) {
                                for (int jewel : move.jewels())
                                {
                                    if (m_jewelState.addToCollected(jewel))
                                    {
                                        onJewelContributionDisabled(jewel);
                                    }
                                }

                                solution.push(move.direction());
                            });

                            if (solveUsingSearchWithBacktracking(solution, newCoords, movesLeft - path.size(), depth + path.size(), additionalMoves, minDepth))
                            {
                                return true;
                            }
                            else
                            {
                                // backtrack
                                Coords2 newCoords = coords;
                                forEachMoveInSolution(path, newCoords, [&](const Move & move, const Coords2 & coords) {
                                    for (int jewel : move.jewels())
                                    {
                                        if (m_jewelState.removeFromCollected(jewel))
                                        {
                                            onJewelContributionEnabled(jewel);
                                        }
                                    }

                                    solution.pop();
                                });
                                return false;
                            }
                        }
                        else
                        {
                            return false;
                        }
                    }

                    continue;
                }

                auto discard = [this, &solution, &move, oldNumJewelsLeft]()
                {
                    for (int jewel : move.jewels())
                    {
                        if (m_jewelState.removeFromCollected(jewel))
                        {
                            onJewelContributionEnabled(jewel);
                        }
                    }

                    m_numJewelsLeftWhenSolvingAt[move.endPos()] = oldNumJewelsLeft;
                    solution.pop();
                };
                solution.push(move.direction());

                for (int jewel : move.jewels())
                {
                    if (m_jewelState.addToCollected(jewel))
                    {
                        onJewelContributionDisabled(jewel);
                    }
                }

                m_numJewelsLeftWhenSolvingAt[end] = m_jewelState.numLeft();

                if (m_jewelState.numLeft() == 0)
                {
                    if (solution.size() > m_level.maxMoves())
                    {
                        if (minDepth > m_level.maxMoves() - additionalMoves * 0.70710678118654752440084436210485f)
                        {
                            discard();
                            return false;
                        }

                        minDepth = depth;
                        JewelState jc = m_jewelState;
                        Solution cpy = solution;
                        while (tryRemoveAnyRunFromSolution(cpy));
                        m_jewelState = jc;
                        if (isSolutionValid(cpy) && cpy.size() <= m_level.maxMoves())
                        {
                            solution = cpy;
                            return true;
                        }
                        else
                        {
                            discard();

                            return false;
                        }
                    }

                    return true;
                }
                else if (movesLeft > -additionalMoves)
                {
                    if (solveUsingSearchWithBacktracking(solution, end, movesLeft - 1, depth + 1, additionalMoves, minDepth))
                    {
                        return true;
                    }
                    else if (movesLeft > 0)
                    {
                        const float skipProbability = 1.0f - (1.0f - m_skipProbabilityAtDepth[depth]) * potential / (maxPotential + 1);
                        if (std::bernoulli_distribution(skipProbability)(m_rng))
                        {
                            discard();
                            return false;
                        }
                    }
                    else if (movesLeft <= 0)
                    {
                        discard();
                        return false;
                    }
                }

                discard();
            }

            return false;
        }

        int countMovesAt(const Coords2 & pos) const
        {
            int c = 0;
            const Moves& moves = m_movesByPosition[pos];
            for (Direction dir : DirectionHelper::values())
            {
                const Move& move = moves[dir];

                if (move.id() >= 0)
                {
                    ++c;
                }
            }

            return c;
        }

        void fillInitialMovePotential()
        {
            const int numEdges = static_cast<int>(m_allMoves.size());

            for (int edgeId = 0; edgeId < numEdges; ++edgeId)
            {
                const Move& move = *(m_allMoves[edgeId]);
                for (const int jewelId : move.jewels())
                {
                    m_potentialOfJewelAtEdge[jewelId][edgeId] = maxPotential;
                }
            }
        }

        void printAllMovesFlat() const
        {
            for (const Move* movePtr : m_allMoves)
            {
                const Move& move = *movePtr;
                const Coords2& start = move.startPos();
                const Coords2& end = move.endPos();
                if (end != start && end != Coords2(0, 0))
                {
                    g_logger.log("(", start.x, ", ", start.y, ") -> (", end.x, ", ", end.y, "): ");
                    for (int id : move.jewels())
                    {
                        g_logger.log(id, ' ');
                    }
                    g_logger.log(" [", m_totalPotentialAtEdge[move.id()], "]");
                }
                g_logger.log('\n');
            }
        }

        void printPotential()
        {
            const int numEdges = static_cast<int>(m_allMoves.size());

            g_logger.log("    ");
            for (int edgeId = 0; edgeId < numEdges; ++edgeId)
            {
                g_logger.log(std::setw(3), edgeId, ' ');
            }
            g_logger.log('\n');

            for (int jewelId = 0; jewelId < numJewels(); ++jewelId)
            {
                g_logger.log(std::setw(2), jewelId, ": ");
                for (int edgeId = 0; edgeId < numEdges; ++edgeId)
                {
                    g_logger.log(std::setw(3), static_cast<int>(m_potentialOfJewelAtEdge[jewelId][edgeId]), ' ');
                }
                g_logger.log('\n');
            }
        }

        // when the jewel is collected for the first time
        void onJewelContributionEnabled(int jewelId)
        {
            const int numEdges = static_cast<int>(m_allMoves.size());

            for (int edgeId = 0; edgeId < numEdges; ++edgeId)
            {
                m_totalPotentialAtEdge[edgeId] += m_potentialOfJewelAtEdge[jewelId][edgeId];
            }
        }

        // when the was collected but is no more
        void onJewelContributionDisabled(int jewelId)
        {
            const int numEdges = static_cast<int>(m_allMoves.size());

            for (int edgeId = 0; edgeId < numEdges; ++edgeId)
            {
                m_totalPotentialAtEdge[edgeId] -= m_potentialOfJewelAtEdge[jewelId][edgeId];
            }
        }

        void summarizeMovePotential()
        {
            const int numEdges = static_cast<int>(m_allMoves.size());

            for (int jewelId = 0; jewelId < numJewels(); ++jewelId)
            {
                for (int edgeId = 0; edgeId < numEdges; ++edgeId)
                {
                    if (DirectionHelper::isDiagonal(m_allMoves[edgeId]->direction()))
                    {
                        // empirical tests show that this is a good factor for diagonal moves
                        m_potentialOfJewelAtEdge[jewelId][edgeId] *= 0.70710678118654752440084436210485f; // 1/sqrt(2)
                    }
                    m_totalPotentialAtEdge[edgeId] += m_potentialOfJewelAtEdge[jewelId][edgeId];
                }
            }
        }

        void propagatePotentialFromJewel(int jewelId)
        {
            std::queue<const Move*> movesQueue;
            for (const Move* move : m_movesCollectingJewel[jewelId])
            {
                movesQueue.emplace(move);
            }

            while (!movesQueue.empty())
            {
                const Move& move = *movesQueue.front();
                movesQueue.pop();

                const Coords2& start = move.startPos();

                const auto& currentPotential = m_potentialOfJewelAtEdge[jewelId][move.id()];
                const auto newPotential = saturatePotential(currentPotential);

                for (const Move* moveAtEnd : m_movesByEndPosition[start])
                {
                    auto& currentPotentialAtEnd = m_potentialOfJewelAtEdge[jewelId][moveAtEnd->id()];
                    if (newPotential > currentPotentialAtEnd)
                    {
                        currentPotentialAtEnd = newPotential;
                        movesQueue.emplace(moveAtEnd);
                    }
                }
            }
        }

        void propagateMovePotential()
        {
            constexpr int numAllPrints = 20;
            const int numEdges = static_cast<int>(m_allMoves.size());

            int numPrints = 0;
            for (int jewelId = 0; jewelId < numJewels(); ++jewelId)
            {
                propagatePotentialFromJewel(jewelId);
                if (jewelId >= numJewels() / numAllPrints * numPrints)
                {
                    g_logger.log("Propagate potential ", jewelId + 1, '/', numJewels(), '\n');
                    ++numPrints;
                }
            }
        }

        PotentialType saturatePotential(PotentialType p) const
        {
            // empirically chosen
            return p / 7 * 3;
        }

        void initializeMovePotential()
        {
            const int numEdges = static_cast<int>(m_allMoves.size());

            m_potentialOfJewelAtEdge = Array2<PotentialType>(numJewels(), numEdges);
            m_totalPotentialAtEdge = std::vector<TotalPotentialType>(numEdges);
        }

        int countJewels() const
        {
            int count = 0;
            m_level.board().forEach([&count](CellType cell, int x, int y) {
                if (cell == CellType::Jewel)
                {
                    ++count;
                }
                });
            return count;
        }

        void printAllMoves() const
        {
            forEach(m_movesByPosition, [](const Moves & moves, int x, int y) {
                const Coords2 start(x, y);

                g_logger.log("From (", x, ", ", y, ") to: ");
                for (Direction dir : DirectionHelper::values())
                {
                    const Move& move = moves[dir];
                    const Coords2& end = move.endPos();
                    if (end != start && end != Coords2(0, 0))
                    {
                        g_logger.log("(", end.x, ", ", end.y, ", {");
                        for (int id : move.jewels())
                        {
                            g_logger.log(id, ' ');
                        }
                        g_logger.log("}) ");
                    }
                }
                g_logger.log('\n');
                });
        }

        void identifyJewels()
        {
            int nextJewelId = 0;
            m_level.board().forEach([this, &nextJewelId](CellType cell, int x, int y) {
                if (cell == CellType::Jewel)
                {
                    m_jewelIdByPosition[x][y] = nextJewelId++;
                }
                });
        }

        void printSccs() const
        {
            const int width = m_sccIdAt.width();
            const int height = m_sccIdAt.height();

            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    const int id = m_sccIdAt[x][y];
                    if (id < 0)
                    {
                        g_logger.log(' ');
                    }
                    else
                    {
                        g_logger.log(id);
                    }
                    g_logger.log(' ');
                }
                g_logger.log('\n');
            }

            for (int i = 0; i < m_sccs.size(); ++i)
            {
                g_logger.log("SCC ", m_sccs[i].id, " has ", m_sccs[i].jewels.size(), " jewels\n");
                g_logger.log("  Neighbours: ");
                for (int n : m_sccs[i].neighbours)
                {
                    g_logger.log(n, ' ');
                }
                g_logger.log('\n');
            }
        }

        void assignJewelsToSccs()
        {
            // there is a possibility that a jewel is collectible from 2 or more SCCs
            // #######
            // #   * #
            // *+#   #
            // * .* O#
            // #*O *##
            // #######

            // if a jewels lies on the edge that moves between different sccs (but not on the start) then
            // it is assigned to the latter scc
            // if it doesn't cross sccs then it is assigned to both (not at once, but we can be sure that
            // end node has outcoming edges to the same scc so it will be handled)

            // use a set to remove duplicates, later copy to vector
            std::vector<std::set<int>> jewels(m_sccs.size());
            int numTotalJewels = 0;
            const int numJewels = m_jewelState.numJewels();
            for (const Move* movePtr : m_allMoves)
            {
                const Move& move = *movePtr;
                if (move.id() < 0)
                {
                    continue;
                }

                const Coords2& v = move.startPos();
                const int iv = m_sccIdAt[v];
                const Coords2& w = move.endPos();
                const int iw = m_sccIdAt[w];
                for (const int jewelId : move.jewels())
                {
                    if (iv != iw && m_jewelIdByPosition[move.startPos()] != jewelId)
                    {
                        jewels[iw].emplace(jewelId);
                    }
                    else
                    {
                        jewels[iv].emplace(jewelId);
                    }

                    ++numTotalJewels;
                }
            }

            for (int i = 0; i < m_sccs.size(); ++i)
            {
                m_sccs[i].jewels = std::vector<JewelId>(std::begin(jewels[i]), std::end(jewels[i]));
            }

            m_lastSccWithJewel.resize(numJewels);
            for (int i = 0; i < m_sccs.size(); ++i)
            {
                for (const int jewelId : m_sccs[i].jewels)
                {
                    m_lastSccWithJewel[jewelId] = i;
                }
            }

            resetSccCountsPerJewel();
        }

        bool mayBeSolvable() const
        {
            // try looking for sccs pair that cannot be both reached in one traversal
            // but both contain the only instances of some jewel
            // ie. look for places where taking one required scc prevents us from taking another required one

            for (const auto& scc : m_sccs)
            {
                for (const int jewelId : scc.jewels)
                {
                    if (m_numSccsWithJewel[jewelId] == 1)
                    {
                        // this is the only scc where we can collect it

                        for (int otherSccId = 0; otherSccId < m_sccs.size(); ++otherSccId)
                        {
                            if (scc.id == otherSccId)
                            {
                                continue;
                            }

                            if (m_ifSccTraversedThenSccUnreachable[scc.id][otherSccId])
                            {
                                // we can't reach this scc if we take the previously checked one

                                for (const int jewelId : m_sccs[otherSccId].jewels)
                                {
                                    // but we have to reach it to have a solution
                                    // so it is not solvable
                                    if (m_numSccsWithJewel[jewelId] == 1)
                                    {
                                        return false;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            return true;
        }

        bool remainsSolvableAfterEnteringScc(std::vector<std::uint8_t> & isTraversed, int i)
        {
            // this may be very costly potentially
            // we'll have to se whether it requires any more early exits

            if (i == invalidSccId)
            {
                return true;
            }

            if (isTraversed[i])
            {
                // already traversed, no action required
                return true;
            }

            bool remainsSolvable = true;
            for (const auto& scc : m_sccs)
            {
                if (m_ifSccTraversedThenSccUnreachable[i][scc.id])
                {
                    for (const int jewelId : scc.jewels)
                    {
                        m_numSccsWithJewel[jewelId] -= 1;
                        if (m_numSccsWithJewel[jewelId] <= 0)
                        {
                            remainsSolvable = false;
                        }
                    }
                }
            }

            for (const auto& scc : m_sccs)
            {
                if (m_ifSccTraversedThenSccUnreachable[i][scc.id])
                {
                    for (const int jewelId : scc.jewels)
                    {
                        m_numSccsWithJewel[jewelId] += 1;
                    }
                }
            }

            return remainsSolvable;
        }

        void fillBackwardSccUnreachabilityIfSccTraversed(const Scc & initialScc)
        {
            // identifies backward reachable sccs from scc

            const int numSccs = m_sccs.size();
            std::vector<int> isVisited(numSccs, false);
            std::stack<int> reachable;
            reachable.emplace(initialScc.id);

            while (!reachable.empty())
            {
                const Scc& scc = m_sccs[reachable.top()];
                reachable.pop();

                m_ifSccTraversedThenSccUnreachable[initialScc.id][scc.id] = false;

                for (const int n : scc.predecessors)
                {
                    if (!isVisited[n])
                    {
                        isVisited[n] = true;
                        reachable.emplace(n);
                    }
                }
            }
        }

        void fillForwardSccUnreachabilityIfSccTraversed(const Scc & initialScc)
        {
            // identifies forward reachable sccs from scc

            const int numSccs = m_sccs.size();
            std::vector<int> isVisited(numSccs, false);
            std::stack<int> reachable;
            reachable.emplace(initialScc.id);

            while (!reachable.empty())
            {
                const Scc& scc = m_sccs[reachable.top()];
                reachable.pop();

                m_ifSccTraversedThenSccUnreachable[initialScc.id][scc.id] = false;

                for (const int n : scc.neighbours)
                {
                    if (!isVisited[n])
                    {
                        isVisited[n] = true;
                        reachable.emplace(n);
                    }
                }
            }
        }

        void fillSccUnreachabilityIfSccTraversed(const Scc & initialScc)
        {
            // identifies reachable sccs from scc

            fillForwardSccUnreachabilityIfSccTraversed(initialScc);
            fillBackwardSccUnreachabilityIfSccTraversed(initialScc);
        }

        void fillSccConditionalUnreachability()
        {
            const int numSccs = m_sccs.size();

            // we will make reachable all the ones we can reach, unreachable by default
            m_ifSccTraversedThenSccUnreachable = Array2<bool>(numSccs, numSccs, true);

            for (const auto& scc : m_sccs)
            {
                fillSccUnreachabilityIfSccTraversed(scc);
            }
        }

        bool isVertex(const Coords2 & v) const
        {
            return m_nodeIdByPosition[v] != invalidNodeId;
        }

        void resetSccCountsPerJewel()
        {
            m_numSccsWithJewel.resize(m_jewelState.numJewels());
            std::fill(std::begin(m_numSccsWithJewel), std::end(m_numSccsWithJewel), 0);

            for (const auto& scc : m_sccs)
            {
                for (const int jewelId : scc.jewels)
                {
                    m_numSccsWithJewel[jewelId] += 1;
                }
            }
        }

        void identifyOneScc(std::stack<Coords2> & S, const Coords2 & v, int& index, Array2<int> & indexAt, Array2<int> & lowlinkAt, Array2<bool> & onStackAt)
        {
            // Set the depth index for v to the smallest unused index
            indexAt[v] = index;
            lowlinkAt[v] = index;
            index += 1;
            S.emplace(v);
            onStackAt[v] = true;

            // Consider successors of v
            const Moves& moves = m_movesByPosition[v];
            for (Direction dir : DirectionHelper::values())
            {
                const Move& move = moves[dir];
                if (move.id() < 0)
                {
                    continue;
                }

                const Coords2& w = move.endPos();
                if (indexAt[w] == -1)
                {
                    // Successor w has not yet been visited; recurse on it
                    identifyOneScc(S, w, index, indexAt, lowlinkAt, onStackAt);
                    lowlinkAt[v.x][v.y] = std::min(lowlinkAt[v.x][v.y], lowlinkAt[w]);
                }
                else if (onStackAt[w])
                {
                    // Successor w is in stack S and hence in the current SCC
                    // If w is not on stack, then (v, w) is a cross-edge in the DFS tree and must be ignored
                    // Note: The next line may look odd - but is correct.
                    // It says w.index not w.lowlink; that is deliberate and from the original paper
                    lowlinkAt[v.x][v.y] = std::min(lowlinkAt[v.x][v.y], indexAt[w]);
                }
            }

            // If v is a root node, pop the stack and generate an SCC
            if (lowlinkAt[v.x][v.y] == indexAt[v])
            {
                std::vector<Coords2> scc;
                Coords2 w;
                do
                {
                    w = S.top();
                    S.pop();
                    onStackAt[w] = false;
                    scc.emplace_back(w);
                } while (w != v);

                m_sccs.emplace_back();
                m_sccs.back().nodes = std::move(scc);
            }
        }

        void identifySccs()
        {
            // https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm

            int index = 0;
            std::stack<Coords2> S;
            Array2<int> indexAt(m_level.width(), m_level.height(), -1);
            Array2<int> lowlinkAt(m_level.width(), m_level.height(), -1);
            Array2<bool> onStackAt(m_level.width(), m_level.height(), false);
            const int width = m_movesByPosition.width();
            const int height = m_movesByPosition.height();

            for (int x = 0; x < width; ++x)
            {
                for (int y = 0; y < height; ++y)
                {
                    const Coords2 v(x, y);
                    if (isVertex(v) && indexAt[v] == -1)
                    {
                        identifyOneScc(S, v, index, indexAt, lowlinkAt, onStackAt);
                    }
                }
            }

            // reverse sccs so they are in topological order
            std::reverse(std::begin(m_sccs), std::end(m_sccs));

            // fills all other information based on sccs
            // ids
            int i = 0;
            for (auto& scc : m_sccs)
            {
                scc.id = i;
                for (const auto& v : scc.nodes)
                {
                    m_sccIdAt[v] = i;
                }
                ++i;
            }

            // neighbours, predecessors
            int iv = 0;
            for (auto& scc : m_sccs)
            {
                std::set<int> neighbours;
                for (const auto& v : scc.nodes)
                {
                    const Moves& moves = m_movesByPosition[v];
                    for (Direction dir : DirectionHelper::values())
                    {
                        const Move& move = moves[dir];
                        if (move.id() < 0)
                        {
                            continue;
                        }

                        const Coords2& w = move.endPos();
                        const int iw = m_sccIdAt[w];

                        if (iv != iw)
                        {
                            scc.bridges.emplace_back(&move);
                            neighbours.emplace(iw);
                        }
                    }
                }

                scc.neighbours = std::vector<SccId>(std::begin(neighbours), std::end(neighbours));
                for (const int n : scc.neighbours)
                {
                    m_sccs[n].predecessors.emplace_back(scc.id);
                }

                ++iv;
            }
        }

        void fillDistancesFromNode(int s, const std::vector<SmallVector<NodeId, 8>> & moveEnds, std::vector<NodeId> & Q)
        {
            // bfs since we have all edge weights equal

            // using a fixed length vector is faster than std::queue
            // we can do it since we know the amount of nodes and we visit each one at most once

            const int numNodes = m_distanceFromTo.width();
            std::vector<std::uint8_t> visited(numNodes, false);
            m_distanceFromTo[s][s] = 0;
            Q[0] = s;
            auto begin = std::begin(Q);
            auto end = begin + 1;
            visited[s] = true;
            while (begin != end)
            {
                int v = *begin;
                ++begin;
                for (const NodeId endV : moveEnds[v])
                {
                    if (!visited[endV])
                    {
                        visited[endV] = true;
                        // distance should never be intmax here so we can safely increment
                        m_distanceFromTo[s][endV] = m_distanceFromTo[s][v] + 1;
                        *end = endV;
                        ++end;
                    }
                }
            }
        }

        void fillDistancesBetweenNodes()
        {
            std::vector<SmallVector<NodeId, 8>> moveEnds(m_nodePositionById.size());

            forEach(m_movesByPosition, [this, &moveEnds](const Moves & moves, int x, int y)
                {
                    const int nodeId = m_nodeIdByPosition[x][y];

                    if (nodeId == invalidNodeId)
                    {
                        return;
                    }

                    for (Direction dir : DirectionHelper::values())
                    {
                        const Move& move = moves[dir];
                        if (move.id() < 0) continue;

                        const Coords2& end = move.endPos();
                        const int endV = m_nodeIdByPosition[end];
                        moveEnds[nodeId].emplace_back(endV);
                    }
                });

            std::vector<NodeId> Q(m_nodePositionById.size());
            const int numNodes = m_distanceFromTo.width();
            for (int i = 0; i < numNodes; ++i)
            {
                fillDistancesFromNode(i, moveEnds, Q);
            }
        }

        void computePairwiseNodeDistances()
        {
            // fills pairwise distances

            Array2<bool> visited(m_level.width(), m_level.height(), false);

            int c = 0;
            for (const Move* movePtr : m_allMoves)
            {
                const Move& move = *movePtr;
                // we use end instead of start because each node has a way to get to
                const Coords2& end = move.endPos();
                if (!visited[end])
                {
                    m_nodeIdByPosition[end] = c;
                    ++c;
                    visited[end] = true;
                }
            }

            if (m_nodeIdByPosition[m_vehicleCoords] == invalidNodeId)
            {
                // can happen if isVehicleSpotAHole == false
                m_nodeIdByPosition[m_vehicleCoords] = c;
                ++c;
            }

            m_distanceFromTo = Array2<DistanceType>(c, c, infiniteDistance);
            m_nodePositionById = std::vector<Coords2>(c);

            m_level.board().forEach([this](CellType cell, int x, int y) {
                const int id = m_nodeIdByPosition[x][y];
                if (id < 0)
                {
                    return;
                }
                m_nodePositionById[id] = Coords2(x, y);
                });

            fillDistancesBetweenNodes();
            g_logger.log("Number of nodes: ", c, '\n');
        }

        bool areAllJewelsReachable() const
        {
            return m_jewelState.numJewels() == countReachableJewels();
        }

        int countReachableJewels() const
        {
            std::vector<std::uint8_t> isReachable(m_jewelState.numJewels(), false);
            for (const Move* move : m_allMoves)
            {
                for (const int jewelId : move->jewels())
                {
                    isReachable[jewelId] = true;
                }
            }
            return std::count(std::begin(isReachable), std::end(isReachable), true);
        }

        void addMove(Move & move)
        {
            const Coords2& end = move.endPos();

            move.setId(static_cast<int>(m_allMoves.size()));

            m_allMoves.emplace_back(&move);
            for (int jewelId : move.jewels())
            {
                m_movesCollectingJewel[jewelId].emplace_back(&move);
            }

            m_movesByEndPosition[end].emplace_back(&move);
        }

        void addMoves(Moves && movesP, const Coords2 & start, std::queue<Coords2> & ends)
        {
            Moves& moves = m_movesByPosition[start] = std::move(movesP);
            for (Direction dir : DirectionHelper::values())
            {
                Move& move = moves[dir];
                const Coords2& start = move.startPos();
                const Coords2& end = move.endPos();

                if (start == end)
                {
                    continue;
                }

                addMove(move);

                // add the destination point to the queue
                // so we gather all moves from there later
                ends.push(end);
            }
        }

        void generateAllMoves()
        {
            const int width = m_level.width();
            const int height = m_level.height();
            Array2<bool> isVisited(width, height, false);

            std::queue<Coords2> coordsQueue;
            coordsQueue.push(m_vehicleCoords);

            while (!coordsQueue.empty())
            {
                const Coords2 start = coordsQueue.front();
                coordsQueue.pop();

                if (isVisited[start])
                {
                    // prevent infinite recursion
                    continue;
                }

                isVisited[start] = true;

                addMoves(generateMovesAt(start), start, coordsQueue);
            }
        }

        Moves generateMovesAt(const Coords2 & start) const
        {
            Moves moves{};

            for (Direction dir : DirectionHelper::values())
            {
                const Coords2 offset = DirectionHelper::offset(dir);
                Coords2 end = start;

                // walk to the nearest stopping point
                std::vector<JewelId> jewels;
                if (m_level[start] == CellType::Jewel)
                {
                    jewels.emplace_back(m_jewelIdByPosition[start]);
                }
                for (;;)
                {
                    end += offset;

                    const int x = end.x;
                    const int y = end.y;
                    const CellType cell = m_level[x][y];

                    if (cell == CellType::Jewel)
                    {
                        jewels.emplace_back(m_jewelIdByPosition[x][y]);
                    }

                    if (cell == CellType::Wall)
                    {
                        // we have to stop one before
                        moves[dir] = Move(invalidMoveId, start, end - offset, std::move(jewels));
                        break;
                    }

                    if (cell == CellType::Mine)
                    {
                        // invalidate the whole direction
                        moves[dir] = Move(invalidMoveId, start, start, std::move(jewels));
                        break;
                    }

                    // there's always a hole under the vehicle
                    if (cell == CellType::Hole || (cell == CellType::Vehicle && isVehicleSpotAHole))
                    {
                        // stop right here
                        moves[dir] = Move(invalidMoveId, start, end, std::move(jewels));
                        break;
                    }
                }
            }

            return moves;
        }
    };
}

// Currently most of the configuration (including maximum time taken by certain
// algorithm parts) can only be specified in the source code by changing the values
// of constexpr variable in class Solver.

int main(int argc, char* argv[])
{
    apto::Bench bench;

    apto::Level level = apto::read<apto::Level>(std::cin);
    if (argc > 1)
    {
        char* end;
        const int maxMoves = std::strtol(argv[1], &end, 10);
        level.setMaxMoves(maxMoves);
    }
    if (apto::g_logger.enabled) write(level, std::cout);

    apto::Solver solver(level, bench);
    auto solution = solver.solve();
    // apto::g_logger.log("NPS: ", static_cast<std::uint64_t>(bench.nodesPerSecond()), '\n');
    apto::g_logger.log("Time: ", static_cast<float>(bench.elapsed().count()) / 1e9, "s\n");
    apto::g_logger.log(solution.size(), '\n');
    write(solution, std::cout);
}