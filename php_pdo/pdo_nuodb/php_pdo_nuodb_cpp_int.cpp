/****************************************************************************
 * Copyright (c) 2012, NuoDB, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of NuoDB, Inc. nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NUODB, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


extern "C" {
#include "php.h"
#ifdef ZEND_ENGINE_2
# include "zend_exceptions.h"
#endif
#include "php_ini.h"
#include "ext/standard/info.h"
#include "pdo/php_pdo.h"
#include "pdo/php_pdo_driver.h"
#include "php_pdo_nuodb.h"
}

#include "php_pdo_nuodb_cpp_int.h"
#include "php_pdo_nuodb_int.h"

using namespace nuodb::sqlapi;

// private
void PdoNuoDbHandle::deleteConnection()
{
    if (_con == NULL) return;
    delete _con;
    _con = NULL;
}

// private
void PdoNuoDbHandle::deleteEnvrionment()
{
    if (_env == NULL) return;
    delete _env;
    _env = NULL;
}

PdoNuoDbHandle::PdoNuoDbHandle(SqlOptionArray *options) : _env(NULL), _con(NULL), _last_stmt(NULL)
{
    setOptions(options);
}

PdoNuoDbHandle::~PdoNuoDbHandle()
{
    deleteConnection();
    deleteEnvrionment();
    deleteOptions();
}

void PdoNuoDbHandle::deleteOptions() {
    // TODO: Fix this later
    if (_opts == NULL) return;
    for (int i=0; i<4; i++) {
        if (_opt_arr[i].option != NULL) {
            delete _opt_arr[i].option;
            _opt_arr[i].option = NULL;
        }
        if (_opt_arr[i].extra != NULL) {
            delete (char *)_opt_arr[i].extra;
            _opt_arr[i].extra = NULL;
        }
    }
    delete _opts;
    _opts = NULL;
}

void PdoNuoDbHandle::setOptions(SqlOptionArray *options)
{
    // TODO: Fix this later
    deleteOptions();
    _opts = new SqlOptionArray;
    _opts->count = 4;
    _opts->array = _opt_arr;
    _opt_arr[0].option = (char const *) strdup(options->array[0].option);
    _opt_arr[0].extra = (void *) strdup((const char *)options->array[0].extra);
    _opt_arr[1].option = (char const *) strdup(options->array[1].option);
    _opt_arr[1].extra = (void *) strdup((const char *)options->array[1].extra);
    _opt_arr[2].option = (char const *) strdup(options->array[2].option);
    _opt_arr[2].extra = (void *) strdup((const char *)options->array[2].extra);
    _opt_arr[3].option = (char const *) strdup(options->array[3].option);
    _opt_arr[3].extra = (void *) strdup((const char *)options->array[3].extra);

}

SqlConnection *PdoNuoDbHandle::createConnection()
{
    deleteConnection();
    deleteEnvrionment();
    _env = SqlEnvironment::createSqlEnvironment(_opts);
    _con = _env->createSqlConnection(_opts);
    return _con;
}

SqlConnection *PdoNuoDbHandle::getConnection()
{
    return _con;
}

PdoNuoDbStatement *PdoNuoDbHandle::createStatement(char const *sql)
{
    PdoNuoDbStatement *rval = NULL;
    if (sql == NULL) return NULL;
    rval = new PdoNuoDbStatement(this);
    rval->createStatement(sql);
    _last_stmt = rval;
    return rval;
}

void PdoNuoDbHandle::closeConnection() {
    if (_con == NULL) return;
    delete _con;
    _con = NULL;
    delete _env;
    _env=NULL;
}

void PdoNuoDbHandle::commit() {
    if (_con == NULL) return;
    _con->commit();
}

void PdoNuoDbHandle::rollback() {
    if (_con == NULL) return;
    _con->rollback();
}







PdoNuoDbStatement::PdoNuoDbStatement(PdoNuoDbHandle *dbh) : _dbh(dbh), _stmt(NULL), _rs(NULL) {
    // empty
}

PdoNuoDbStatement::~PdoNuoDbStatement() {
    if (_rs != NULL) delete _rs;
    _rs = NULL;
    if (_stmt != NULL) delete _stmt;
    _stmt = NULL;
}

SqlPreparedStatement *PdoNuoDbStatement::createStatement(char const *sql) {
    if (sql == NULL) return NULL;
    SqlConnection *_con = NULL;
    _con = _dbh->getConnection();
    if (_con == NULL) return NULL;
    _stmt = _con->createPreparedStatement(sql);
    _rs = NULL;
    return _stmt;
}

void PdoNuoDbStatement::execute() {
    if (_stmt == NULL) return;
    _stmt->execute();
}

void PdoNuoDbStatement::executeQuery() {
    if (_stmt == NULL) return;
    _rs = _stmt->executeQuery();
}

bool PdoNuoDbStatement::hasResultSet() {
    return (_rs != NULL);
}

bool PdoNuoDbStatement::next() {
    if (_rs == NULL) return false;
    return _rs->next();
}

size_t PdoNuoDbStatement::getColumnCount() {
    if (_rs == NULL) return false;
    return _rs->getColumnCount();
}

char const * PdoNuoDbStatement::getColumnName(size_t column) {
    char const *rval = NULL;
    if (_rs == NULL) return NULL;
    try {
        SqlColumnMetaData *scmd = _rs->getMetaData(column+1);
        rval = scmd->getColumnName();
        delete scmd;
    } catch (ErrorCodeException &e) {
        printf("Failed: %s", e.what());
    }
    return rval;
}

int PdoNuoDbStatement::getSqlType(size_t column) {
    if (_rs == NULL) return 0;
    SqlColumnMetaData *scmd = _rs->getMetaData(column+1);
    SqlType t = scmd->getType();
    delete scmd;
    switch(t) {
        case SQL_BOOLEAN:
            return PDO_NUODB_SQLTYPE_BOOLEAN;
        case SQL_INTEGER:
            return PDO_NUODB_SQLTYPE_INTEGER;
        case SQL_BIGINT:
            return PDO_NUODB_SQLTYPE_BIGINT;
        case SQL_DOUBLE:
            return PDO_NUODB_SQLTYPE_DOUBLE;
        case SQL_STRING:
            return PDO_NUODB_SQLTYPE_STRING;
        case SQL_DATE:
            return PDO_NUODB_SQLTYPE_DATE;
        case SQL_TIME:
            return PDO_NUODB_SQLTYPE_TIME;
        case SQL_DATETIME:
            return PDO_NUODB_SQLTYPE_DATETIME;
    }
    return 0;
}

char const *PdoNuoDbStatement::getString(size_t column) {
    if (_rs == NULL) return NULL;
    return _rs->getString(column+1);
}

unsigned int PdoNuoDbStatement::getInteger(size_t column) {
    if (_rs == NULL) return 0;
    return _rs->getInteger(column+1);
}

unsigned long PdoNuoDbStatement::getLong(size_t column) {
    if (_rs == NULL) return 0;
    return _rs->getLong(column+1);
}