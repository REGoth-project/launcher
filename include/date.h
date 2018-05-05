/*  Copyright (C) 2018 The REGoth Team
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DATE_H
#define DATE_H
#include <cstdint>
#include <stdexcept>
#include <string>
#include <regex>

namespace date
{

class Date
{
public:
    Date(std::int32_t year, std::uint8_t month, std::uint8_t day,
      std::uint8_t hour, std::uint8_t minute, std::uint8_t second)
    {
        if(month < 1 || month > 12)
        {
            throw std::runtime_error("Invalid month");
        }
        int daysPerMonth;
        switch(month)
        {
            case 4:
            case 6:
            case 9:
            case 11:
            daysPerMonth = 30;
            break;
            case 2:
            daysPerMonth = isLeapYear(year) ? 28 : 29;
            break;
            default:
            daysPerMonth = 31;
            break;
        }

        if(day < 1 || day > daysPerMonth)
        {
            throw std::runtime_error("Invalid day");
        }

        if(hour < 0 || hour > 23)
        {
            throw std::runtime_error("Invalid hour");
        }

        if(minute < 0 || minute > 59)
        {
            throw std::runtime_error("Invalid minute");
        }

        if(second < 0 || second > 59)
        {
            throw std::runtime_error("Invalid second");
        }

        m_year = year;
        m_month = month;
        m_day = day;
        m_hour = hour;
        m_minute = minute;
        m_second = second;
    }

    std::int32_t getYear() const { return m_year; }
    std::uint8_t getMonth() const { return m_month; }
    std::uint8_t getDay() const { return m_day; }
    std::uint8_t getHour() const { return m_hour; }
    std::uint8_t getMinute() const { return m_minute; }
    std::uint8_t getSecond() const { return m_second; }

    static bool isLeapYear(std::int32_t year)
    {
        // See: https://www.mathsisfun.com/leap-years.html
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    }

    static Date fromIsoDatetime(const std::string& s)
    {
        const auto format = "(\\d{4})-(\\d{2})-(\\d{2})T(\\d{2}):(\\d{2}):(\\d{2})Z";
        std::regex re(format);
        std::smatch base_match;

        if (std::regex_match(s, base_match, re)) 
        {
            return Date(
              std::atoi(base_match[1].str().c_str()),
              std::atoi(base_match[2].str().c_str()),
              std::atoi(base_match[3].str().c_str()),
              std::atoi(base_match[4].str().c_str()),
              std::atoi(base_match[5].str().c_str()),
              std::atoi(base_match[6].str().c_str()));
        }
        else
        {
            throw std::runtime_error("Invalid date string");
        }
    }

private:
    std::int32_t m_year;
    std::uint8_t m_month, m_day, m_hour, m_minute, m_second;
};

bool operator==(const Date& lhs, const Date& rhs)
{
  return lhs.getDay() == rhs.getDay() &&
        lhs.getHour() == rhs.getHour() &&
        lhs.getMinute() == rhs.getMinute() &&
        lhs.getMonth() == rhs.getMonth() &&
        lhs.getSecond() == rhs.getSecond() &&
        lhs.getYear() == rhs.getYear();
}

bool operator!=(const Date& lhs, const Date& rhs)
{
  return !(lhs == rhs);
}

bool operator>(const Date& lhs, const Date& rhs)
{
    return lhs.getYear() > rhs.getYear() ||
          lhs.getMonth() > rhs.getMonth() ||
          lhs.getDay() > rhs.getDay() ||
          lhs.getHour() > rhs.getHour() ||
          lhs.getMinute() > rhs.getMinute() ||
          lhs.getSecond() > rhs.getSecond();
}

bool operator<(const Date& lhs, const Date& rhs)
{
    return rhs > lhs;
}

bool operator>=(const Date& lhs, const Date& rhs)
{
    return !(lhs < rhs);
}

bool operator<=(const Date& lhs, const Date& rhs)
{
    return !(lhs > rhs);
}

}

#endif
